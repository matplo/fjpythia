#include <fjpythia/util/fjutils.h>
#include <fastjet/contrib/SoftDrop.hh>

namespace fj = fastjet;

namespace FJUtils
{
	std::vector<fj::PseudoJet> soft_drop_jets(std::vector<fj::PseudoJet> jets, double z_cut, double beta, double Rjet)
	{
		// http://fastjet.hepforge.org/svn/contrib/contribs/RecursiveTools/tags/1.0.0/example_softdrop.cc
		// give the soft drop groomer a short name
		// Use a symmetry cut z > z_cut R^beta
		// By default, there is no mass-drop requirement
		fj::contrib::SoftDrop sd(beta, z_cut, Rjet);
		// cout << "SoftDrop groomer is: " << sd.description() << endl;

		std::vector<fj::PseudoJet> soft_dropped;
		for (unsigned ijet = 0; ijet < jets.size(); ijet++)
		{
				// Run SoftDrop and examine the output
				fj::PseudoJet sd_jet   = sd(jets[ijet]);
				// cout << endl;
				// cout << "original    jet: " << jets[ijet] << endl;
				// cout << "SoftDropped jet: " << sd_jet << endl;

				assert(sd_jet      != 0); //because soft drop is a groomer (not a tagger), it should always return a soft-dropped jet

				// cout << "  delta_R between subjets: " << sd_jet.structure_of<fj::contrib::SoftDrop>().delta_R() << endl;
				// cout << "  symmetry measure(z):     " << sd_jet.structure_of<fj::contrib::SoftDrop>().symmetry() << endl;
				// cout << "  mass drop(mu):           " << sd_jet.structure_of<fj::contrib::SoftDrop>().mu() << endl;

				sd_jet.set_user_index(ijet);
				soft_dropped.push_back(sd_jet);
		}
		return soft_dropped;
	}

	std::vector<fj::PseudoJet> getPseudoJetsFromPythia(Pythia8::Pythia *pythia, bool only_final)
	{
		std::vector<fj::PseudoJet> parts;
		for (unsigned int ip = 0; ip < pythia->event.size(); ip++)
		{
			if (pythia->event[ip].isFinal())
			{
				fj::PseudoJet psj;
				psj.reset_momentum(pythia->event[ip].px(), pythia->event[ip].py(), pythia->event[ip].pz(), pythia->event[ip].e());
				psj.set_user_index(ip);
				PythiaUserInfo *uinfo = new PythiaUserInfo(&pythia->event, ip);
				psj.set_user_info(uinfo);
				parts.push_back(psj);
			}
		}
		return parts;
	}
}
