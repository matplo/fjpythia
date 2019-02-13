#include <fjpythia/util/argparser.h>

#include <iostream>

namespace FJPyUtil
{
	ArgParser::ArgParser()
	: tokens()
	, regopts()
	{
		;
	}

	ArgParser::~ArgParser()
	{
		if (isSet("--debug"))
		{
			std::cout << "[i] known options:" << std::endl;
			std::vector<std::string>::const_iterator itr;
			for (auto st : this->regopts)
			{
				std::cout << "    " << st << std::endl;
			}
			std::cout << "    ... done." << std::endl;
		}
	}
}
