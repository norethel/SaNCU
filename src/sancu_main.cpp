/*
 * sancu_main.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: norethel
 */

#include <sndfile.hh>
#include <iostream>
#include <cstring>

const std::string VOICE_FILE = "/home/norethel/wav_files/voice.wav";
const std::string NOISE_FILE = "/home/norethel/wav_files/noise.wav";
const std::string MIXED_FILE = "/home/norethel/wav_files/mixed.wav";

const size_t BUFFER_SIZE = 1024;

static double voice_buffer[BUFFER_SIZE];
static double noise_buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
	SndfileHandle voice_file(VOICE_FILE, SFM_READ,
	        SF_FORMAT_WAV | SF_FORMAT_PCM_16, 1, 16000);

	SndfileHandle noise_file(NOISE_FILE, SFM_READ,
	        SF_FORMAT_WAV | SF_FORMAT_PCM_16, 1, 16000);

	SndfileHandle mixed_file(MIXED_FILE, SFM_WRITE,
	        SF_FORMAT_WAV | SF_FORMAT_PCM_16, 1, 16000);

	size_t voice_read_bytes = 0;
	size_t noise_read_bytes = 0;

	do
	{
		voice_read_bytes = voice_file.read(voice_buffer, BUFFER_SIZE);
		noise_read_bytes = noise_file.read(noise_buffer, BUFFER_SIZE);

		for (size_t i = 0; i < voice_read_bytes; ++i)
		{
			voice_buffer[i] += noise_buffer[i];
		}

		mixed_file.write(voice_buffer, voice_read_bytes);
	}
	while (voice_read_bytes > 0 && noise_read_bytes > 0);

	return 0;
}
