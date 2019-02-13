#ifndef FJPYTHIA_UTIL_FJUTILS_HH
#define FJPYTHIA_UTIL_FJUTILS_HH

#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequence.hh>
#include <fastjet/contrib/SoftDrop.hh>
#include <Pythia8/Pythia.h>
#include <vector>

namespace FJUtils
{
	// not some default values(!)
	std::vector<fastjet::PseudoJet> soft_drop_jets(std::vector<fastjet::PseudoJet> jets,
	                                               double z_cut = 0.1,
	                                               double beta = 0.0,
	                                               double Rjet = 1.0);

	class PythiaUserInfo : public fastjet::PseudoJet::UserInfoBase
	{
	public:
		PythiaUserInfo()
			: fastjet::PseudoJet::UserInfoBase::UserInfoBase(), fPythia(0), fEvent(0), fIndex(-1)
		{
			;
		}

		PythiaUserInfo(Pythia8::Pythia *p, int x)
			: fastjet::PseudoJet::UserInfoBase::UserInfoBase(), fPythia(p), fEvent(0), fIndex(x)
		{
			fEvent = &(fPythia->event);
		}

		PythiaUserInfo(Pythia8::Event *ev, int x)
			: fastjet::PseudoJet::UserInfoBase::UserInfoBase(), fPythia(0), fEvent(ev), fIndex(x)
		{
			;
		}

		virtual ~PythiaUserInfo()
		{
			;
		}

		int getIndex() const
		{
			return fIndex;
		}

		void setIndex(int x)
		{
			fIndex = x;
		}

		Pythia8::Pythia *getPythia() const
		{
			return fPythia;
		}

		void setPythia(Pythia8::Pythia *p)
		{
			fPythia = p;
		}

		Pythia8::Event *getEvent() const
		{
			return fEvent;
		}

		void setEvent(Pythia8::Event *ev)
		{
			fEvent = ev;
		}

		Pythia8::Particle *getParticle() const
		{
			if (fEvent)
				return &(fEvent->at(fIndex));
			return 0x0;
		}

	private:
		Pythia8::Pythia *fPythia;
		Pythia8::Event  *fEvent;
		int 			 fIndex;
	};

	std::vector<fastjet::PseudoJet> getPseudoJetsFromPythia(Pythia8::Pythia *pythia, bool only_final = true);
}

#endif
