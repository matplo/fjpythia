#ifndef FJPYTHIA_UTIL_PYTHIAUTILS_HH
#define FJPYTHIA_UTIL_PYTHIAUTILS_HH

#include <vector>

namespace Pythia8
{
	class Pythia;
	class Particle;
}

namespace PythiaUtils
{
	bool has_mother(const Pythia8::Pythia &pythia, Pythia8::Particle *p, int pid);
	bool is_from_mother_2body_decay(const Pythia8::Pythia &pythia, Pythia8::Particle *p, int pid);

	std::vector<int> find_outgoing_hard_electrons(Pythia8::Pythia *pythia);
	std::vector<int> find_outgoing_beam_remnants(Pythia8::Pythia *pythia);

	void cook_pythia_settings(Pythia8::Pythia *pythia);
}

#endif
