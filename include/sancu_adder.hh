/*
 * sancu_adder.hh
 *
 *  Created on: Mar 17, 2014
 *      Author: norethel
 */

#ifndef SANCU_ADDER_HH_
#define SANCU_ADDER_HH_

#include <string>
#include <list>
#include <vector>

class SancuAdder
{
	public:

	SancuAdder(const std::string& _script_file);

	~SancuAdder() {}

	void execute();

	private:

	void parse_snr(std::string& _line);
	void parse_voice(std::ifstream& fscript);
	void parse_noise(std::ifstream& fscript);
	void parse_result(std::ifstream& fscript);
	void parse_script_file();

	void read_files(SndfileHandle& voice_file, SndfileHandle& noise_file);

	std::string script_file;
	std::list<std::string> voice_files;
	std::list<std::string> noise_files;
	std::list<std::string> result_files;
	std::list<double> snr_levels;

	std::vector<double*> voice_signal;
	std::vector<double*> noise_signal;

	double Esig;
	double Enoise;

	double snr_ratio;

	static const double SNR_CONST_RATIO = 10.0;
	static const double BUFFER_SIZE = 1024;
};

#endif /* SANCU_ADDER_HH_ */
