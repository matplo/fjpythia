#include <fjpythia/util/pyutils.h>
#include <fjpythia/util/argparser.h>
#include <fjpythia/util/strutil.h>

#include <Pythia8/Pythia.h>
#include <TString.h>

#include <iostream>
using namespace std;

namespace PythiaUtils
{
	std::vector<int> find_outgoing_hard_electrons(Pythia8::Pythia *pythia)
	{
		std::vector<int> v;
		for (int i = 0; i < pythia->event.size(); i++)
		{
			if (!pythia->event[i].isFinal()) continue;
			if (pythia->event[i].id() == 11)
			{
				// http://home.thep.lu.se/~torbjorn/pythia81html/ParticleProperties.html
				// 21 - 29 : particles of the hardest subprocess
				// 23 : outgoing
				// 44 : outgoing shifted by a branching
				// if (pythia->event[i].status() == 23 || pythia->event[i].status() == 44)

				// 91 - 99 : particles produced in decay process, or by Bose-Einstein effects
				// 91 : normal decay products
				if (pythia->event[i].status() != 91 || pythia->event[i].status() == 63) // not from a decay or outgoing beam remnant
				{
					v.push_back(i);
				}
			}
		}
		return v;
	}

	void cook_pythia_settings(Pythia8::Pythia *pythia)
	{
		auto &args = FJPyUtil::ArgParser::Instance();

		// settings from a cmnd file?
		if (args.isSet("--pythia-config"))
		{
			pythia->readFile(args.getOpt("--pythia-config").c_str());
			args.addOpts("--pythia-process-configured");
		}

		int nDebugEvents = args.getOptInt("--debug-events", 0);
		pythia->readString(TString::Format("Next:numberShowEvent=%d", nDebugEvents).Data());
		pythia->readString(TString::Format("Next:numberShowInfo=%d", nDebugEvents).Data());
		pythia->readString(TString::Format("Next:numberShowProcess=%d", nDebugEvents).Data());
		pythia->readString(TString::Format("Next:numberCount=%d", nDebugEvents).Data());

		pythia->readString("Stat:showProcessLevel=on");

		if (args.isSet("--hardQCD"))
		{
			pythia->readString("HardQCD:all=on");
			args.addOpts("--pythia-process-configured");
		}

		if (args.isSet("--eic-dis") or args.isSet("--eic-lowQ2") or
		    args.isSet("--eic-cgamma") or args.isSet("--eic-bgamma") or args.isSet("--eic-qgamma") or
		    args.isSet("--eic-test"))
		{
			pythia->readString("Beams:idA=11");
			pythia->readString("Beams:idB=2212");
			pythia->readString("Beams:eA=20");
			pythia->readString("Beams:eB=250");
			pythia->readString("Beams:frameType=2");
			pythia->readString("Init:showChangedSettings=on");
			pythia->readString("Main:timesAllowErrors=10000");
			if (args.isSet("--eic-dis"))
			{
				pythia->readString("WeakBosonExchange:ff2ff(t:gmZ)=on");
				pythia->readString("PhaseSpace:Q2Min=10");
				pythia->readString("SpaceShower:pTmaxMatch=2");
				pythia->readString("PDF:lepton=off");
				pythia->readString("TimeShower:QEDshowerByL=off");

				args.addOpts("--pythia-process-configured");
			}
			if (args.isSet("--eic-lowQ2"))
			{
				pythia->readString("HardQCD:all=on");
				pythia->readString("PDF:lepton2gamma=on");
				pythia->readString("Photon:Q2max=1.");
				pythia->readString("Photon:Wmin=10.");
				pythia->readString("PhaseSpace:pTHatMin=2.");
				pythia->readString("PhotonParton:all=on");
				pythia->readString("Photon:ProcessType=0");

				args.addOpts("--pythia-process-configured");
			}
			if (args.isSet("--eic-cgamma"))
			{
				pythia->readString("PDF:lepton2gamma=on");
				pythia->readString("PhotonParton:ggm2ccbar=on");

				args.addOpts("--pythia-process-configured");
			}

			if (args.isSet("--eic-bgamma"))
			{
				pythia->readString("PDF:lepton2gamma=on");
				pythia->readString("PhotonParton:ggm2bbbar=on");

				args.addOpts("--pythia-process-configured");
			}

			if (args.isSet("--eic-qgamma"))
			{
				pythia->readString("PDF:lepton2gamma=on");
				pythia->readString("PhotonParton:qgm2qgm=on");

				args.addOpts("--pythia-process-configured");
			}

			if (args.isSet("--eic-test"))
			{
				pythia->readString("WeakBosonExchange:ff2ff(t:gmZ)=on");
				pythia->readString("PhaseSpace:Q2Min=1");
				pythia->readString("SpaceShower:pTmaxMatch=2");
				pythia->readString("PDF:lepton=off");
				pythia->readString("TimeShower:QEDshowerByL=off");

				pythia->readString("HardQCD:all=on");
				pythia->readString("PDF:lepton2gamma=on");
				pythia->readString("Photon:Q2max=1.");
				pythia->readString("Photon:Wmin=10.");
				pythia->readString("PhaseSpace:pTHatMin=1.");
				pythia->readString("PhaseSpace:pTHatMax=18.");
				pythia->readString("PhotonParton:all=on");
				pythia->readString("Photon:ProcessType=0");

				args.addOpts("--pythia-process-configured");

			}
		}

		// settings from a command line?
		std::string pythiaOpt = args.getOpt("--pythia");
		auto pyopts = StrUtil::split_to_vector(pythiaOpt.c_str(), ",");
		for (auto o : pyopts)
		{
			StrUtil::replace_substring(o, "_", " ");
			pythia->readString(o.c_str());
			args.addOpts("--pythia-process-configured");
		}

		if (args.isSet("--time-seed"))
		{
			pythia->readString("Random:setSeed=on");
			pythia->readString("Random:seed=0");
		}

		if (!args.isSet("--pythia-process-configured"))
		{
			cout << "[w] this will likely fail. enable a process with a flag or --pythia list,of,settings,..." << endl;
			args.addOpts("--debug");
		}

	}
}
