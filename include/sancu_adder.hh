/*
 * sancu_adder.hh
 *
 *  Created on: Mar 17, 2014
 *      Author: norethel
 */

#ifndef SANCU_ADDER_HH_
#define SANCU_ADDER_HH_

#include <string>
#include <vector>

class SancuSignal;

class SancuAdder
{
	public:

	SancuAdder(const std::string& _script_file);

	~SancuAdder();

	void execute();

	private:

	void parse_snr(std::string& _line);
	void parse_voice(std::ifstream& fscript);
	void parse_noise(std::ifstream& fscript);
	void parse_result(std::ifstream& fscript);
	void parse_output_path(std::string& _line);
	void parse_script_file();

	void prepare_data();

	void recalculate_data();

	void modify_path(std::string& path, size_t noise_num, size_t snr_num);

	std::string script_file;
	std::string output_path;

	std::vector<std::string> voice_files;
	std::vector<std::string> noise_files;

	std::vector<double> snr_levels;
	std::vector<double> snr_ratios;

	std::vector<SancuSignal*> voice_signals;
	std::vector<SancuSignal*> noise_signals;

	static const double SNR_CONST_RATIO = 10.0;
};

#endif /* SANCU_ADDER_HH_ */
