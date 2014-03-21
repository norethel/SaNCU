#include "sancu_signal.hh"
#include <cmath>
#include <iostream>
#include <sndfile.hh>
#include "sancu_sample.hh"

const size_t SancuSignalChunk::BUF_LEN = 1024;

double compute_energy(const double* _buffer, const size_t& _length,
        const double& _mean)
{
	double energy = 0;

	for (size_t i = 0; i < _length; ++i)
	{
		energy += std::pow((_buffer[i] - _mean), 2.0);
	}

	return energy;
}

SancuSignalChunk::SancuSignalChunk(const SancuSignalChunk& _chunk) :
		length(_chunk.length)
{
	buffer = new double[BUF_LEN];

	for (size_t i = 0; i < length; ++i)
	{
		buffer[i] = _chunk.buffer[i];
	}
}

SancuSignal::SancuSignal(const std::string& _path, const bool& _compute_energy) :
		path(_path), energy(0)
{
	read_signal(_compute_energy);
}

SancuSignal::SancuSignal(const SancuSignal& _signal) :
		path(_signal.path), energy(_signal.energy), mean(_signal.mean), format(
		        _signal.format), channels(_signal.channels), samplerate(
		        _signal.samplerate)
{
	std::vector<SancuSignalChunk*>::const_iterator iter =
	        _signal.chunks.begin();

	for (; iter != _signal.chunks.end(); ++iter)
	{
		chunks.push_back(new SancuSignalChunk(**iter));
	}
}

SancuSignal::~SancuSignal()
{
	std::vector<SancuSignalChunk*>::iterator iter = chunks.begin();

	for (; iter != chunks.end(); ++iter)
	{
		delete *iter;
	}
}

SancuSignal& SancuSignal::operator+=(const SancuSample& _sample)
{
	std::vector<SancuSignalChunk*>::iterator iter = chunks.begin();
	std::vector<TSampleChunk>::const_iterator sample_iter =
	        _sample.chunks.begin();

	for (; iter != chunks.end() && sample_iter != _sample.chunks.end();
	        ++iter, ++sample_iter)
	{
		size_t length = (*iter)->length;
		double* buffer1 = (*iter)->buffer;
		double* buffer2 = sample_iter->first->buffer;

		for (size_t i = 0; i < length; ++i)
		{
			buffer1[i] += buffer2[i];
		}
	}

	return *this;
}

SancuSignal& SancuSignal::operator*=(const double& snr_level)
{
	std::vector<SancuSignalChunk*>::iterator iter = chunks.begin();

	for (; iter != chunks.end(); ++iter)
	{
		double* buffer = (*iter)->buffer;
		size_t length = (*iter)->length;

		for (size_t i = 0; i < length; ++i)
		{
			buffer[i] *= snr_level;
		}
	}

	return *this;
}

void SancuSignal::write_back()
{
	std::vector<SancuSignalChunk*>::iterator iter = chunks.begin();
	SndfileHandle file(path, SFM_WRITE, format, channels, samplerate);

	for (; iter != chunks.end(); ++iter)
	{
		SancuSignalChunk* chunk = *iter;

		file.write(chunk->buffer, chunk->length);
	}
}

void SancuSignal::compute_mean()
{
	double sum = 0;
	size_t total = 0;
	std::vector<SancuSignalChunk*>::iterator iter = chunks.begin();

	for (; iter != chunks.end(); ++iter)
	{
		for (size_t i = 0; i < (*iter)->length; ++i)
		{
			sum += (*iter)->buffer[i];
		}

		total += (*iter)->length;
	}

	mean = sum / static_cast<double>(total);
}

void SancuSignal::compute_energy()
{
	std::vector<SancuSignalChunk*>::iterator iter = chunks.begin();
	energy = 0;

	for (; iter != chunks.end(); ++iter)
	{
		energy += ::compute_energy((*iter)->buffer, (*iter)->length, mean);
	}
}

void SancuSignal::normalize()
{
	std::vector<SancuSignalChunk*>::iterator iter = chunks.begin();
	double min = 0;
	double max = 0;
	const double a = -1.0;
	const double b = 1.0;

	for (; iter != chunks.end(); ++iter)
	{
		for (size_t i = 0; i < (*iter)->length; ++i)
		{
			min = (min > (*iter)->buffer[i]) ? (*iter)->buffer[i] : min;
			max = (max < (*iter)->buffer[i]) ? (*iter)->buffer[i] : max;
		}
	}

	for (; iter != chunks.end(); ++iter)
	{
		for (size_t i = 0; i < (*iter)->length; ++i)
		{
			(*iter)->buffer[i] = a
			        + (((*iter)->buffer[i] - min) * (b - a)) / (max - min);
		}
	}
}

void SancuSignal::read_signal(const bool& _compute_energy)
{
	SndfileHandle file(path);
	size_t read_bytes = 0;
	size_t total_bytes = 0;
	double sum = 0;

	format = file.format();
	channels = file.channels();
	samplerate = file.samplerate();

	std::cout << "start reading file: " << path.c_str() << std::endl;
	std::cout << "normalization is: " << file.command(SFC_GET_NORM_DOUBLE, 0, 0)
	        << std::endl;

	do
	{
		double* buffer = new double[SancuSignalChunk::BUF_LEN];

		read_bytes = file.read(buffer, SancuSignalChunk::BUF_LEN);
		total_bytes += read_bytes;

		if (read_bytes > 0)
		{
			if (_compute_energy)
			{
				for (size_t i = 0; i < read_bytes; ++i)
				{
					sum += buffer[i];
				}
			}

			chunks.push_back(new SancuSignalChunk(buffer, read_bytes));
		}
		else
		{
			delete[] buffer;
		}
	}
	while (read_bytes > 0);

	if (_compute_energy)
	{
		mean = sum / static_cast<double>(total_bytes);

		compute_energy();
	}
}
