#include <fjpythia/eic/example.h>
#include <fjpythia/util/argparser.h>
#include <fjpythia/util/strutil.h>
#include <fjpythia/util/fjutils.h>

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

int fj_and_root()
{
	auto &args = FJPyUtil::ArgParser::Instance();

	// open an output file
	std::string foutname = args.getOpt("--out", "default_output.root");
	TFile fout(foutname.c_str(), "recreate");
	// book histograms
	TH1F hjetpt("hjetpt", "p_{T}^{jet} (GeV/c)", 50, 0, 50);
	// book some ntuples
	TNtuple tne("tne", "tne", "n:pid:xsec");
	TNtuple tnj("tnj", "tnj", "pid:xsec:pt:phi:eta:sdpt:sdphi:sdeta:sdDR:zg:sdmu");

	// intialize PYTHIA
	Pythia pythia;
	pythia.readString("Next:numberShowEvent=0");

	// main06.cc is a part of the PYTHIA event generator.
	// setup some default settings - if nothing specified by the user
	if (!args.isSet("--pythia-config") and !args.isSet("--pythia"))
	{
		// TString sdemo = TString("--pythia PDF:lepton=off,WeakSingleBoson:ffbar2gmZ=on,23:onMode=off,23:onIfAny=1_2_3_4_5,Beams:idA=11,Beams:idB=-11,Beams:eCM=91.188,");
		// double mZ = pythia.particleData.m0(23);
		// TString smZ = TString::Format("Beams:eCM=%.5f", mZ);
		// sdemo += smZ;
		TString sdemo = TString("--pythia Beams:frameType=2,Beams:idA=2212,Beams:idB=11,Beams:eA=250,Beams:eB=20,WeakBosonExchange:ff2ff(t:gmZ)=on,PhaseSpace:Q2Min=10,SpaceShower:pTmaxMatch=2,PDF:lepton=off,TimeShower:QEDshowerByL=off");
		args.addOpts(sdemo.Data());
	}
	args.dump();
	// settings from a cmnd file?
	if (args.isSet("--pythia-config"))
	{
		pythia.readFile(args.getOpt("--pythia-config").c_str());
	}
	// settings from a command line?
	std::string pythiaOpt = args.getOpt("--pythia");
	auto pyopts = StrUtil::split_to_vector(pythiaOpt.c_str(), ",");
	for (auto o : pyopts)
	{
		StrUtil::replace_substring(o, "_", " ");
		pythia.readString(o.c_str());
	}
	if (!pythia.init())
	{
		cout << "[e] pythia init failed." << endl;
		return -1;
	}

	// generate and analyze events
	int nEv = args.getOptInt("--nev", 1); // default will be 1 event(!)
	double jetR = args.getOptDouble("--jetR", 0.4);
	double minJetPt = args.getOptDouble("--minJetPt", 0.0);
	double maxPartEta = std::abs(args.getOptDouble("--maxParticleEta", 10.));

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
	for (int iEvent = 0; iEvent < nEv; ++iEvent)
	{
		if (!pythia.next()) continue;
		tne.Fill(iEvent, pythia.info.code(), pythia.info.sigmaGen());

		auto parts = FJUtils::getPseudoJetsFromPythia(&pythia);
		std::vector<fj::PseudoJet> parts_selected = partSelector(parts);

		// run jet finding
		fj::JetDefinition jet_def(fj::antikt_algorithm, jetR);
		fj::ClusterSequence ca(parts_selected, jet_def);
		auto jets = jetSelector(ca.inclusive_jets());

		// soft drop jets
		std::vector<fj::PseudoJet> sdjets = FJUtils::soft_drop_jets(jets, 0.1, 0.0, jetR); // note, running with default soft drop

		fj::contrib::SoftDrop sd(0.1, 0.0, 0.4); //(beta, z_cut, Rjet);
		// write jet properties to an ntuple
		for (unsigned int ij = 0; ij < jets.size(); ij++)
		{
			hjetpt.Fill(jets[ij].perp());
			fj::PseudoJet sd_jet = sd(jets[ij]);
			// cout << endl;
			// cout << "original    jet: " << jets[ijet] << endl;
			// cout << "SoftDropped jet: " << sd_jet << endl;

			assert(sd_jet      != 0); //because soft drop is a groomer (not a tagger), it should always return a soft-dropped jet

			// std::cout << "  delta_R between subjets: " << sd_jet.structure_of<fj::contrib::SoftDrop>().delta_R() << std::endl;
			// std::cout << "  symmetry measure(z):     " << sd_jet.structure_of<fj::contrib::SoftDrop>().symmetry() << std::endl;
			// std::cout << "  mass drop(mu):           " << sd_jet.structure_of<fj::contrib::SoftDrop>().mu() << std::endl;
			tnj.Fill(pythia.info.code(), pythia.info.sigmaGen(),
			         jets[ij].perp(), jets[ij].phi(), jets[ij].eta(),
			         sdjets[ij].perp(), sdjets[ij].phi(), sdjets[ij].eta(),
			         sdjets[ij].structure_of<fj::contrib::SoftDrop>().delta_R(),
			         sdjets[ij].structure_of<fj::contrib::SoftDrop>().symmetry(),
			         sdjets[ij].structure_of<fj::contrib::SoftDrop>().mu());
		}
	}
	// write and close the output file
	fout.Write();
	fout.Close();
	cout << "[i] file written: " << fout.GetName();
	// Done.
	return 0;
}
