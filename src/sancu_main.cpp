#include <iostream>
#include "sancu_adder.hh"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Please, provide script file!" << std::endl;
		std::cout << "example: ./sancu script_file.snc" << std::endl;

		return -1;
	}

	SancuAdder adder(argv[1]);

	adder.execute();

	return 0;
}
