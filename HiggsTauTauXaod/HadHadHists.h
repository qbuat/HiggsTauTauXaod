#ifndef HiggsTauTauXaod_HadHadHists_H
#define HiggsTauTauXaod_HadHadHists_H


#include <map>
#include "TH1F.h"

#include "xAODTau/TauJet.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMissingET/MissingET.h"

#include <EventLoop/Worker.h>

class HadHadHists 
{

 public:
  
  // this is a standard constructor
  HadHadHists (const std::string & name);
  virtual ~ HadHadHists() {};

  void book();

  void fill_evt(const xAOD::EventInfo *ei);
  void fill_tau(const xAOD::TauJet * tau1, const xAOD::TauJet* tau2, const double & weight=1.0);
  void fill_met(const xAOD::MissingET * met);

  void record(EL::Worker* wk);

 private:

  std::map<std::string, TH1F*> m_h1d;
  std::string m_name;

};

#endif
