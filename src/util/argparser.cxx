#include <fjpythia/util/argparser.h>
#include <fjpythia/util/strutil.h>

#include <iostream>

namespace FJPyUtil
{
	ArgParser::ArgParser()
	: tokens()
	, regopts()
	{
		;
	}

	void ArgParser::addOpts(const char *what)
	{
		auto sv = StrUtil::split_to_vector(what, " ");
		for (auto s : sv)
		{
			this->tokens.push_back(s);
		}
	}


	void ArgParser::dump()
	{
		std::cout << "[i] args are:" << std::endl;
		for (auto s : this->tokens)
			std::cout << "    " << s << std::endl;
	}

	const int ArgParser::getOptInt(const char* option, int idefault)
	{
		if (isSet(option))
			return StrUtil::str_to_int(getOpt(option).c_str(), idefault);
		return idefault;
	}

	const double ArgParser::getOptDouble(const char* option, double ddefault)
	{
		if (isSet(option))
			return StrUtil::str_to_double(getOpt(option).c_str(), ddefault);
		return ddefault;
	}

	ArgParser::~ArgParser()
	{
		if (isSet("--debug") || isSet("--help") || isSet("-h"))
		{
			std::cout << "[i] known options:" << std::endl;
			std::vector<std::string>::const_iterator itr;
			for (auto st : this->regopts)
			{
				std::cout << "    " << st << std::endl;
			}
		}
	}
}
