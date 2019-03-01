#include <fjpythia/eic/example.h>
#include <fjpythia/util/argparser.h>
#include <fjpythia/util/strutil.h>
#include <fjpythia/util/fjutils.h>
#include <fjpythia/util/looputil.h>
#include <fjpythia/util/pyutils.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include <TTree.h>
#include <TNtuple.h>

#include <fastjet/Selector.hh>
#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequence.hh>
#include <fastjet/contrib/SoftDrop.hh>
namespace fj = fastjet;

#include "Pythia8/Pythia.h"
using namespace Pythia8;

#include <TString.h>

namespace mateusz
{
int fj_and_root()
{
	auto &args = FJPyUtil::ArgParser::Instance();
	args.dump();

	// open an output file
	std::string foutname = args.getOpt("--out", "default_output.root");
	TFile fout(foutname.c_str(), "recreate");
	// book histograms
	TH1F hjetpt("hjetpt", "p_{T}^{jet} (GeV/c)", 50, 0, 50);
	TH2F hlund("hlund", "hlund;log(1/#theta);log(k_{T});density", 60, 0, 6, 180, -9, 9);
	// book some ntuples
	TNtuple tne("tne", "tne", "n:procid:xsec");
	// particles
	TNtuple tnp("tnp", "tnp", "procid:xsec:pt:phi:eta:m:pid:status");
	// jets
	TNtuple tnj("tnj", "tnj", "procid:xsec:pt:phi:eta:lpt:lpid:lstatus:sdpt:sdphi:sdeta:sdDR:zg:sdmu");

	// intialize PYTHIA
	Pythia pythia;
	PythiaUtils::cook_pythia_settings(&pythia);
	if (!pythia.init())
	{
		cout << "[e] pythia init failed." << endl;
		return -1;
	}

	// generate and analyze events
	int nEv = args.getOptInt("--nev", 1); // default will be 1 event(!)
	double jetR = args.getOptDouble("--jetR", 0.4);
	double minJetPt = args.getOptDouble("--minJetPt", 0.0);
	double maxPartEta = std::abs(args.getOptDouble("--maxParticleEta", 20.));

	// dump some parameters of the analysis
	cout << "[i] configuration: " << endl
		 << "    events:        " << nEv << endl
		 << "    jetR:          " << jetR << endl
		 << "    minJetPt:      " << minJetPt << endl
		 << "    maxPartEta:    " << maxPartEta << endl
		 << "    output:        " << foutname << endl;

	fj::Selector partSelector = fastjet::SelectorAbsEtaMax(maxPartEta);
	fj::Selector jetSelector = fastjet::SelectorAbsEtaMax(maxPartEta - jetR - 0.01) * fastjet::SelectorPtMin(minJetPt);

	// Begin event loop. Generate event. Skip if error. List first few.
	LoopUtil::TPbar pbar(nEv);
	for (int iEvent = 0; iEvent < nEv; ++iEvent)
	{
		pbar.Update();
		if (!pythia.next()) continue;
		tne.Fill(iEvent, pythia.info.code(), pythia.info.sigmaGen());

		auto parts = FJUtils::getPseudoJetsFromPythia(&pythia, true); // only_final==true
		std::vector<fj::PseudoJet> parts_selected = partSelector(parts);

		// get the beam scattered electrons
		auto hard_electron_indexes = PythiaUtils::find_outgoing_hard_electrons(&pythia);

		for (auto &psj : parts_selected)
		{
			Pythia8::Particle *_p = psj.user_info<FJUtils::PythiaUserInfo>().getParticle();
			tnp.Fill(pythia.info.code(), pythia.info.sigmaGen(),
			         psj.perp(), psj.phi(), psj.eta(), _p->m(), _p->id(), _p->status());
		}

		FJUtils::mask_momentum_of(hard_electron_indexes, parts_selected);

		// run jet finding
		fj::JetDefinition jet_def(fj::antikt_algorithm, jetR);
		fj::ClusterSequence ca(parts_selected, jet_def);
		std::vector<fj::PseudoJet> jets_inclusive = ca.inclusive_jets();
		std::vector<fj::PseudoJet> jets = jetSelector(jets_inclusive);

		// soft drop jets
		std::vector<fj::PseudoJet> sdjets = FJUtils::soft_drop_jets(jets, 0.1, 0.0, jetR); // note, running with default soft drop

		// lund declustering
		FJUtils::lund_decluster(jets);

		// write jet properties to an ntuple
		for (unsigned int ij = 0; ij < jets.size(); ij++)
		{
			hjetpt.Fill(jets[ij].perp());
			if (jets[ij].has_user_info<FJUtils::LundJetInfo>())
			{
				auto lsplits = jets[ij].user_info<FJUtils::LundJetInfo>();
				for (auto &lsplit : lsplits.splits())
					hlund.Fill(log(1./lsplit.dR), log(lsplit.pt2 * lsplit.dR));
			}
			auto _lead = fastjet::sorted_by_pt(jets[ij].constituents())[0];
			Pythia8::Particle *_lead_py = _lead.user_info<FJUtils::PythiaUserInfo>().getParticle();
			tnj.Fill(pythia.info.code(), pythia.info.sigmaGen(),
			         jets[ij].perp(), jets[ij].phi(), jets[ij].eta(),
			         // leading particle in the jet - by pT
			         _lead.perp(), _lead_py->id(), _lead_py->status(),
			         // after soft drop
			         sdjets[ij].perp(), sdjets[ij].phi(), sdjets[ij].eta(),
			         sdjets[ij].structure_of<fj::contrib::SoftDrop>().delta_R(),
			         sdjets[ij].structure_of<fj::contrib::SoftDrop>().symmetry(), // aka zg
			         sdjets[ij].structure_of<fj::contrib::SoftDrop>().mu());
		}
	}
	// write and close the output file
	fout.Write();
	fout.Close();

	pythia.stat();

	cout << "[i] file written: " << fout.GetName() << endl;
	// Done.
	return 0;
} // fj_and_root
} // namespace
