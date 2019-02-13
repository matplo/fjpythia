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
namespace fj = fastjet;

#include "Pythia8/Pythia.h"
using namespace Pythia8;

int fj_and_root()
{
	auto &args = FJPyUtil::ArgParser::Instance();

	// main06.cc is a part of the PYTHIA event generator.
	// Generator.
	Pythia pythia;

	//// Allow no substructure in e+- beams: normal for corrected LEP data.
	//pythia.readString("PDF:lepton = off");
	//// Process selection.
	//pythia.readString("WeakSingleBoson:ffbar2gmZ = on");
	//// Switch off all Z0 decays and then switch back on those to quarks.
	//pythia.readString("23:onMode = off");
	//pythia.readString("23:onIfAny = 1 2 3 4 5");

	//// LEP1 initialization at Z0 mass.
	//pythia.readString("Beams:idA =  11");
	//pythia.readString("Beams:idB = -11");
	double mZ = pythia.particleData.m0(23);
	// pythia.settings.parm("Beams:eCM", mZ);
	pythia.readString("Next:numberShowEvent=0");
	cout << mZ;

	if (!args.isSet("--pythia-config") and !args.isSet("--pythia"))
	{
		args.addOpts("--pythia PDF:lepton=off,WeakSingleBoson:ffbar2gmZ=on,23:onMode=off,23:onIfAny=1_2_3_4_5,Beams:idA=11,Beams:idB=-11,Beams:eCM=91.188");
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

	pythia.init();

	// open an output file
	std::string foutname = args.getOpt("--out", "default_output.root");
	TFile fout(foutname.c_str(), "recreate");
	// book histograms
	TH1F hjetpt("hjetpt", "p_{T}^{jet} (GeV/c)", 50, 0, 50);
	// book some ntuples
	TNtuple tne("tne", "tne", "n:pid:xsec");
	TNtuple tnj("tnj", "tnj", "pid:xsec:pt:phi:eta:sdpt:sdphi:sdeta:sdDR:zg:sdmu");

	// generate and analyze events
	int nEv = args.getOptInt("--nev", 1); // default will be 1 event(!)
	double jetR = args.getOptDouble("--jetR", 0.4);
	double minJetPt = args.getOptDouble("--minJetPt", 0.0);
	double maxPartEta = std::abs(args.getOptDouble("--maxParticleEta", 10.));

	cout << "[i] configuration: " << endl
		 << "    events:        " << nEv << endl
		 << "    jetR:          " << jetR << endl
		 << "    minJetPt:      " << minJetPt << endl
		 << "    maxPartEta:    " << maxPartEta << endl;

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
		fj::ClusterSequence ca(parts, jet_def);
		auto jets = jetSelector(ca.inclusive_jets());

		// soft drop jets
		std::vector<fj::PseudoJet> sdjets = FJUtils::soft_drop_jets(jets); // note, running with default soft drop

		// write jet properties to an ntuple
		for (unsigned int ij = 0; ij < jets.size(); ij++)
		{
			hjetpt.Fill(jets[ij].perp());
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
