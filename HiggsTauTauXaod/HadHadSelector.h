#ifndef HiggsTauTauXaod_HadHadSelector_H
#define HiggsTauTauXaod_HadHadSelector_H

#include <EventLoop/Algorithm.h>
#include <TH1F.h>
#include "EventLoop/OutputStream.h"

#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "HiggsTauTauXaod/EventVariablesTool.h"
#include "HiggsTauTauXaod/HadHadHists.h"

#include "GoodRunsLists/GoodRunsListSelectionTool.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "DiTauMassTools/MissingMassTool.h"

namespace Trig {
  class TrigDecisionTool;
}


class HadHadSelector : public EL::Algorithm
{


  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
 public:
  // float cutValue;
  std::vector<std::string> trigger_names;

 private:
  GoodRunsListSelectionTool *m_grl; //!
  Trig::TrigDecisionTool *m_trigDecTool; //!
  EventVariablesTool *m_var_tool; //!
  MissingMassTool *m_mmc_tool;//!

  xAOD::TEvent * event;//!
  xAOD::TStore * store;//!

  HadHadHists m_book;//!

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:

  TH1F* m_hcutflow; //!
  TH1F* m_hmet_diff; //!

 
  // this is a standard constructor
  HadHadSelector ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // this is needed to distribute the algorithm to the workers
  ClassDef(HadHadSelector, 1);
};

#endif
