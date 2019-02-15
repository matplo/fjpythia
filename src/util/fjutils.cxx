#include <fjpythia/util/fjutils.h>
#include <fastjet/contrib/SoftDrop.hh>

namespace fj = fastjet;

#include <iostream>

namespace FJUtils
{
	int leading_pid(const fastjet::PseudoJet &j)
	{
		int lpid = 0;
		auto _lead = fj::SelectorNHardest(1)(j.constituents())[0];
		Pythia8::Particle *_lead_py = _lead.user_info<FJUtils::PythiaUserInfo>().getParticle();
		if (_lead_py)
			lpid = _lead_py->id();
		return lpid;
	}

	Pythia8::Particle * leading_pythia(const fastjet::PseudoJet &j)
	{
		int lpid = 0;
		auto _lead = fj::SelectorNHardest(1)(j.constituents())[0];
		Pythia8::Particle *_lead_py = _lead.user_info<FJUtils::PythiaUserInfo>().getParticle();
		return _lead_py;
	}

	void lund_decluster(std::vector<fastjet::PseudoJet> &jets)
	{
		for (auto & j: jets)
		{
			int nsplits = 0;
			fj::JetDefinition decl_jet_def(fj::cambridge_algorithm, 1.0);
			fj::ClusterSequence decl_ca(j.constituents(), decl_jet_def);
			LundJetInfo *linfo = new LundJetInfo();
			for (const auto & j : decl_ca.inclusive_jets(0))
			{
				fj::PseudoJet jj, j1, j2;
				jj = j;
				while (jj.has_parents(j1,j2))
				{
					nsplits++;
					// make sure j1 is always harder branch
					if (j1.pt2() < j2.pt2()) std::swap(j1,j2);
					// collect info and fill in the histogram
					double delta_R = j1.delta_R(j2);
					auto leading = leading_pythia(j);
					if (leading)
						linfo->addSplit(jj.perp(), jj.e(), j1.perp(), j2.perp(), delta_R, leading->pT(), leading->id());
					else
						linfo->addSplit(jj.perp(), jj.e(), j1.perp(), j2.perp(), delta_R, leading->pT(), leading->id());

					// // double delta_R_norm = delta_R / jet_def.R();
					// double delta_R_norm = delta_R; // MP
					// double z = j2.pt()/(j1.pt() + j2.pt());
					// double y = log(1.0 / delta_R_norm);
					// // there is an ambiguity here: can use z or j2.pt() / j.pt()
					// double lnpt_rel = log(z * delta_R_norm);
					// double lnpt_abs = log(j2.pt()/j.pt() * delta_R_norm);
					// // hists_2d["lund-zrel"].add_entry(y, lnpt_rel, evwgt);
					// // hists_2d["lund-zabs"].add_entry(y, lnpt_abs, evwgt);
					// double lnpt = log(j2.pt() * delta_R);
					// // hists_2d["lund-lnpt"].add_entry(y, lnpt, evwgt);
					// follow harder branch
					jj = j1;
				}
			}
			j.set_user_info(linfo);
		}
	}


	void mask_momentum_of(std::vector<int> idxs, std::vector<fastjet::PseudoJet>& v)
	{
		for (auto idx : idxs)
		{
			// std::cout << idx << std::endl;
			for (unsigned int i = 0; i < v.size(); i++)
			{
				if (v[i].user_index() == idx)
				{
					// std::cout << "--> uidx=" << idx << " : in-v-idx=" << i << std::endl;
					// std::cout << " pt: " << v[i].perp() << std::endl;
					v[i] = v[i] * 1.e-6;
					// v[i].reset_momentum(v[i].px() / v[i].e() / 100.,
					//                         v[i].py() / v[i].e() / 100.,
					//                         v[i].pz() / v[i].e() / 100.,
					//                         v[i].e() / v[i].e() / 100.);
					// std::cout << " pt: " << v[i].perp() << std::endl;
				}
			}
		}
	}

	std::vector<fj::PseudoJet> soft_drop_jets(std::vector<fj::PseudoJet> jets, double z_cut, double beta, double Rjet)
	{
		// http://fastjet.hepforge.org/svn/contrib/contribs/RecursiveTools/tags/1.0.0/example_softdrop.cc
		// give the soft drop groomer a short name
		// Use a symmetry cut z > z_cut R^beta
		// By default, there is no mass-drop requirement
		fj::contrib::SoftDrop sd(beta, z_cut, Rjet);
		// std::cout << "SoftDrop groomer is: " << sd.description() << std::endl;

		std::vector<fj::PseudoJet> soft_dropped;
		for (unsigned ijet = 0; ijet < jets.size(); ijet++)
		{
				// Run SoftDrop and examine the output
				fj::PseudoJet sd_jet   = sd(jets[ijet]);
				// cout << endl;
				// cout << "original    jet: " << jets[ijet] << endl;
				// cout << "SoftDropped jet: " << sd_jet << endl;

				assert(sd_jet      != 0); //because soft drop is a groomer (not a tagger), it should always return a soft-dropped jet

				// std::cout << "  delta_R between subjets: " << sd_jet.structure_of<fj::contrib::SoftDrop>().delta_R() << std::endl;
				// std::cout << "  symmetry measure(z):     " << sd_jet.structure_of<fj::contrib::SoftDrop>().symmetry() << std::endl;
				// std::cout << "  mass drop(mu):           " << sd_jet.structure_of<fj::contrib::SoftDrop>().mu() << std::endl;

				sd_jet.set_user_index(ijet);
				soft_dropped.push_back(sd_jet);
		}
		return soft_dropped;
	}

	std::vector<fj::PseudoJet> getPseudoJetsFromPythia(Pythia8::Pythia *pythia, bool only_final)
	{
		std::vector<fj::PseudoJet> parts;
		for (int ip = 0; ip < pythia->event.size(); ip++)
		{
			if (only_final)
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
			else
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
