#include <fjpythia/eic/example.h>
#include <fjpythia/util/argparser.h>
#include <fjpythia/util/strutil.h>

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

int fj_and_root( int argc, char *argv[] )
{
	auto &args = FJPyUtil::ArgParser::Instance();
	args.addOpts(argc, argv);

	// main06.cc is a part of the PYTHIA event generator.
	// Generator.
	Pythia pythia;

	// Allow no substructure in e+- beams: normal for corrected LEP data.
	pythia.readString("PDF:lepton = off");
	// Process selection.
	pythia.readString("WeakSingleBoson:ffbar2gmZ = on");
	// Switch off all Z0 decays and then switch back on those to quarks.
	pythia.readString("23:onMode = off");
	pythia.readString("23:onIfAny = 1 2 3 4 5");

	// LEP1 initialization at Z0 mass.
	pythia.readString("Beams:idA =  11");
	pythia.readString("Beams:idB = -11");
	double mZ = pythia.particleData.m0(23);
	pythia.settings.parm("Beams:eCM", mZ);
	pythia.init();

	// open an output file
	std::string foutname = args.getOpt("--out", "default_output.root");
	TFile fout(foutname.c_str(), "recreate");
	// book histograms
	TH1F hjetpt("hjetpt", "p_{T}^{jet} (GeV/c)", 50, 0, 50);
	// book some ntuples
	TNtuple tne("tne", "tne", "n:pid:xsec");

	// generate and analyze events
	int nEv = StrUtil::str_to_int(args.getOpt("--nev").c_str(), 1); // default will be 1 event(!)
	cout << "[i] will run for " << nEv << " events." << endl;
	// Begin event loop. Generate event. Skip if error. List first few.
	for (int iEvent = 0; iEvent < nEv; ++iEvent)
	{
		if (!pythia.next()) continue;
		tne.Fill(iEvent, pythia.info.code(), pythia.info.sigmaGen());
	}
	// write and close the output file
	fout.Write();
	fout.Close();
	cout << "[i] file written: " << fout.GetName();
	// Done.
	return 0;
}
