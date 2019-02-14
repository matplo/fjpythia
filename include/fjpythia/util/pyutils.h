#ifndef FJPYTHIA_UTIL_PYTHIAUTILS_HH
#define FJPYTHIA_UTIL_PYTHIAUTILS_HH

#include <vector>

namespace Pythia8
{
	class Pythia;
}

namespace PythiaUtils
{
	std::vector<int> find_outgoing_hard_electrons(Pythia8::Pythia *pythia);
	void cook_pythia_settings(Pythia8::Pythia *pythia);
}

#endif
