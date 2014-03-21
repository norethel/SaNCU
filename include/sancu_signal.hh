/*
 * sancu_signal.hh
 *
 *  Created on: Mar 18, 2014
 *      Author: norethel
 */

#ifndef SANCU_SIGNAL_HH_
#define SANCU_SIGNAL_HH_

#include <vector>
#include <cstddef>
#include <string>

class SancuSample;

double compute_energy(const double* _buffer, const size_t& _length,
        const double& _mean);

class SancuSignalChunk
{
	public:

	static const size_t BUF_LEN;

	double* buffer;
	size_t length;

	SancuSignalChunk(double* const _buffer, const size_t& _length) :
			buffer(_buffer), length(_length)
	{
	}

	SancuSignalChunk(const SancuSignalChunk& _signal);

	~SancuSignalChunk()
	{
		delete[] buffer;
	}

	private:

	SancuSignalChunk();
};

class SancuSignal
{
	public:

	std::string path;
	double energy;
	double mean;

	int format;
	int channels;
	int samplerate;

	std::vector<SancuSignalChunk*> chunks;

	SancuSignal(const std::string& _path, const bool& _compute_energy = false);
	SancuSignal(const SancuSignal& _signal);

	~SancuSignal();

	SancuSignal& operator+=(const SancuSample& _sample);
	SancuSignal& operator*=(const double& snr_level);

	void write_back();

	void compute_mean();
	void compute_energy();

	void normalize();

	private:

	SancuSignal();

	void read_signal(const bool& _compute_energy);
};

#endif /* SANCU_SIGNAL_HH_ */
