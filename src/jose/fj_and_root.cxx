#include <fjpythia/eic/example.h>
#include <fjpythia/util/argparser.h>
#include <fjpythia/util/strutil.h>
#include <fjpythia/util/fjutils.h>
#include <fjpythia/util/looputil.h>
#include <fjpythia/util/pyutils.h>

#include <TH1F.h>
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

namespace jose
{
int fj_and_root()
{
	auto &args = FJPyUtil::ArgParser::Instance();
	args.dump();

	// open an output file
	std::string foutname = args.getOpt("--out", "default_output.root");
	TFile fout(foutname.c_str(), "recreate");
    fout.cd();
        // book histograms
        TH1F h_Q("h_Q", "Q [GeV]", 100, 0., 50.);
        TH1F h_W("h_W", "W [GeV]", 100, 0., 140.);
        TH1F h_x("h_x", "x", 100, 0., 1.);
        TH1F h_y("h_y", "y", 100, 0., 1.);
        TH1F h_nu("h_nu", "nu [GeV]", 100, 0., 10000.);
    
        TH1F h_pTe("h_pTe", "pT of scattered electron", 100, 0., 20.);
    
        TH1F h_pTjets("h_pTjets", "p_{T}^{jet} (GeV/c)", 40, 0., 40.);
        //TH1F h_etaJets("h_etaJets", "eta for jets", 20, -5., 5.);
        //TH1F h_phiJets("h_phiJets", "phi for jets", 30, -M_PI, M_PI);
        //TH1F h_mJets("h_mJets", "mass for jets", 30, 0., 30.);
        //TH1F h_multJets("h_multJets", "multiplicity for jets", 15, 0., 30.);
    
	// book some ntuples
	TNtuple tne("tne", "tne", "n:procid:xsec");
	// particles
	TNtuple tnp("tnp", "tnp", "procid:xsec:pt:phi:eta:m:pid:status");
	// jets
	TNtuple tnj("tnj", "tnj", "procid:xsec:pt:phi:eta:lpt:lpid:lstatus:sdpt:sdphi:sdeta:sdDR:zg:sdmu");

	// intialize PYTHIA
	Pythia pythia;
        Event& event = pythia.event;
    
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
        
            double mProton = event[1].m();
        
            // Four-momenta of proton, electron, virtual photon/Z^0/W^+-.
            Vec4 pProton = event[1].p();
            Vec4 peIn    = event[4].p();
            Vec4 peOut   = event[6].p();
            Vec4 pPhoton = peIn - peOut;
        
            // Q2, W2, Bjorken x, y, nu.
            double Q2  = - pPhoton.m2Calc();
            double W2  = (pProton + pPhoton).m2Calc();
            double x   = Q2 / (2. * pProton * pPhoton);
            double y   = (pProton * pPhoton) / (pProton * peIn);
            double nu  = (pProton * pPhoton) / mProton;
        
            // Fill kinematics histograms.
            h_Q.Fill( sqrt(Q2) );
            h_W.Fill( sqrt(W2) );
            h_x.Fill( x );
            h_y.Fill( y );
            h_nu.Fill( nu );
        
            h_pTe.Fill( event[6].pT() );
        
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

		// write jet properties to an ntuple
		for (unsigned int ij = 0; ij < jets.size(); ij++)
		{
			h_pTjets.Fill(jets[ij].perp());
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
	cout << "[i] file written: " << fout.GetName() << endl;
	// Done.
	return 0;
} // fj_and_root
} // namespace
