/**
 * @file sancu_adder.cpp
 *
 * @date Mar 17, 2014
 * @author norethel
 *
 * @{
 */

#include "sancu_adder.hh"
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstring>
#include <iostream>
#include <sndfile.hh>
#include <sancu_signal.hh>
#include <sancu_sample.hh>

SancuAdder::SancuAdder(const std::string& _script_file) :
		script_file(_script_file)
{
	parse_script_file();
}

SancuAdder::~SancuAdder()
{
	std::vector<SancuSignal*>::iterator voice_sig_iter = voice_signals.begin();
	std::vector<SancuSignal*>::iterator noise_sig_iter = noise_signals.begin();

	for (; voice_sig_iter != voice_signals.end(); ++voice_sig_iter)
	{
		delete *voice_sig_iter;
	}

	for (; noise_sig_iter != noise_signals.end(); ++noise_sig_iter)
	{
		delete *noise_sig_iter;
	}
}

void SancuAdder::parse_snr(std::string& _line)
{
	double snr;
	char* current = strtok(const_cast<char*>(_line.c_str()), ",");

	while (0 != current)
	{
		std::stringstream converter(current);
		converter >> snr;
		snr_levels.push_back(snr);

		current = strtok(0, ",");
	}
}

void SancuAdder::parse_voice(std::ifstream& fscript)
{
	std::string line;

	while (std::getline(fscript, line)
	        && std::string::npos == line.find("</voice>"))
	{
		voice_files.push_back(line);
	}
}

void SancuAdder::parse_noise(std::ifstream& fscript)
{
	std::string line;

	while (std::getline(fscript, line)
	        && std::string::npos == line.find("</noise>"))
	{
		noise_files.push_back(line);
	}
}

void SancuAdder::parse_output_path(std::string& _line)
{
	output_path = _line;

	if ('/' != output_path[output_path.length() - 1])
	{
		output_path += '/';
	}
}

void SancuAdder::parse_script_file()
{
	std::ifstream fscript(script_file.c_str());
	std::string line;

	while (std::getline(fscript, line))
	{
		if (std::string::npos != line.find("<snr>"))
		{
			if (std::getline(fscript, line))
			{
				parse_snr(line);
			}
		}
		else if (std::string::npos != line.find("<voice>"))
		{
			parse_voice(fscript);
		}
		else if (std::string::npos != line.find("<noise>"))
		{
			parse_noise(fscript);
		}
		else if (std::string::npos != line.find("<output_path>"))
		{
			if (std::getline(fscript, line))
			{
				parse_output_path(line);
			}
		}
		else
		{
			std::cout << "parse warning: unknown line:" << std::endl;
			std::cout << "\"" << line.c_str() << "\"" << std::endl;
		}
	}

	fscript.close();
}

void SancuAdder::modify_path(std::string& path, size_t snr_num,
        size_t noise_num)
{
	std::stringstream ss;
	std::string postfix;

	size_t slash_pos = path.find_last_of('/');

	/* long path found */
	if (std::string::npos != slash_pos)
	{
		/* extract only filename */
		path = path.substr(slash_pos + 1);
	}

	path.insert(0, output_path);

	ss << "_" << snr_num << "_" << noise_num;
	ss >> postfix;

	path.insert(path.length() - 4, postfix);
}

void SancuAdder::prepare_data()
{
	std::vector<double>::iterator iter = snr_levels.begin();

	for (; iter != snr_levels.end(); ++iter)
	{
		snr_ratios.push_back(
		        std::pow(SNR_CONST_RATIO, (*iter / SNR_CONST_RATIO)));
	}

	std::vector<std::string>::iterator voice_iter = voice_files.begin();

	for (; voice_iter != voice_files.end(); ++voice_iter)
	{
		voice_signals.push_back(new SancuSignal(*voice_iter, true));
	}

	std::vector<std::string>::iterator noise_iter = noise_files.begin();

	for (; noise_iter != noise_files.end(); ++noise_iter)
	{
		noise_signals.push_back(new SancuSignal(*noise_iter, false));
	}
}

void SancuAdder::recalculate_data()
{
	std::vector<double>::iterator snr_iter = snr_ratios.begin();
	size_t snr_num = 0;

	/* for every snr_ratio */
	for (; snr_iter != snr_ratios.end(); ++snr_iter)
	{
		++snr_num;
		std::vector<SancuSignal*>::iterator noise_iter = noise_signals.begin();
		size_t noise_num = 0;

		/* for every noise */
		for (; noise_iter != noise_signals.end(); ++noise_iter)
		{
			SancuSampleReader noise_reader(*noise_iter);
			++noise_num;

			std::vector<SancuSignal*>::iterator voice_iter =
			        voice_signals.begin();

			/* for every voice signal */
			for (; voice_iter != voice_signals.end(); ++voice_iter)
			{
				/* take signal */
				SancuSignal* voice_sig = new SancuSignal(**voice_iter);

				/* modify output signal path */
				modify_path(voice_sig->path, snr_num, noise_num);

				size_t num_chunks = voice_sig->chunks.size();
				size_t last_chunk_size = voice_sig->chunks.back()->length;

				/* 2. read noise sample of the same length as current voice signal */
				std::vector<TSampleChunk> chunks = noise_reader.read(num_chunks,
				        last_chunk_size);

				/* the constructor also computes the energy of the read noise sample */
				SancuSample noise_sample(chunks);

				/* 3. compute SNR coefficient for current noise level adjustment */
				double snr_level = std::sqrt(
				        voice_sig->energy
				                / ((*snr_iter) * noise_sample.energy));

				/* 4. multiply noise signal by calculated coefficient */
				noise_sample *= snr_level;

				std::cout << "Noise energy before recalculation: "
				        << noise_sample.energy << std::endl;

				noise_sample.compute_mean();
				noise_sample.compute_energy();

				std::cout << "Noise energy after recalculation: "
				        << noise_sample.energy << std::endl;

				/* 5. add noise to voice signal */
				*voice_sig += noise_sample;

				std::cout << "Voice energy: " << voice_sig->energy << std::endl;

				double out_snr = 10
				        * std::log10(voice_sig->energy / noise_sample.energy);

				std::cout << "output SNR: " << out_snr << "dB" << std::endl;

				/* write back the signal */
				voice_sig->write_back();

				delete voice_sig;
			}
		}
	}
}

void SancuAdder::execute()
{
	/* calculate snr_ratios, read voice and noise files;
	 * compute energy for voice files while reading them */
	prepare_data();

	recalculate_data();
}
