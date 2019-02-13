#ifndef FJPYTHIA_UTIL_FJUTILS_HH
#define FJPYTHIA_UTIL_FJUTILS_HH

#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequence.hh>
#include <fastjet/contrib/SoftDrop.hh>

namespace FJUtils
{
	// not some default values(!)
	std::vector<fastjet::PseudoJet> soft_drop_jets(std::vector<fastjet::PseudoJet> jets,
	                                               double z_cut = 0.1,
	                                               double beta = 0.0,
	                                               double Rjet = 1.0);
}

#endif
