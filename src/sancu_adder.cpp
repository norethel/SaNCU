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

SancuAdder::SancuAdder(const std::string& _script_file) :
		script_file(_script_file), Esig(0), Enoise(0), snr_ratio(0)
{
	parse_script_file();
}

void SancuAdder::parse_snr(std::string& _line)
{
	std::stringstream converter;
	double snr;
	char* current;

	_line = _line.substr(5, std::string::npos);

	do
	{
		current = strtok(static_cast<char*>(_line.c_str()), ",");
		converter << current;
		converter >> snr;
		snr_levels.push_back(snr);
	}
	while (0 != current);
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

void SancuAdder::parse_result(std::ifstream& fscript)
{
	std::string line;

	while (std::getline(fscript, line)
	        && std::string::npos == line.find("</result>"))
	{
		result_files.push_back(line);
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
		else if (std::string::npos != line.find("<result>"))
		{
			parse_result(fscript);
		}
	}

	fscript.close();
}

void read_files(SndfileHandle& voice_file, SndfileHandle& noise_file)
{
	size_t voice_read = 0;

	do
	{
		double* voice_buffer = new double[BUFFER_SIZE];
		double* noise_buffer = new double[BUFFER_SIZE];

		voice_read = voice_file.read(voice_buffer, BUFFER_SIZE);
		size_t noise_read = noise_file.read(noise_buffer, BUFFER_SIZE);

		if (noise_read < voice_read)
		{
			noise_file.seek(0, SEEK_SET);
			noise_read = noise_file.read(noise_buffer, BUFFER_SIZE);
		}

		for (size_t i = 0; i < voice_read; ++i)
		{
			Esig += std::pow(voice_buffer[i], 2.0);
			Enoise += std::pow(noise_buffer[i], 2.0);
		}

		voice_signal.push_back(voice_buffer);
		noise_signal.push_back(noise_buffer);
	}
	while (voice_read > 0);
}

void SancuAdder::execute()
{
	std::list<double>::iterator iter = snr_levels.begin();

	while (iter != snr_levels.end())
	{
		std::list<std::string>::iterator noise_iter = noise_files.begin();

		snr_ratio = std::pow(SNR_CONST_RATIO, (*iter / SNR_CONST_RATIO));

		while (noise_iter != noise_files.end())
		{
			std::list<std::string>::iterator voice_iter = voice_files.begin();

			SndfileHandle noise_file(*noise_iter);

			while (voice_iter != voice_files.end())
			{
				SndfileHandle voice_file(*voice_iter);

				read_files(voice_file, noise_file);

				++voice_iter;
			}

			++noise_iter;
		}

		++iter;
	}
}
