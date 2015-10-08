#ifndef HiggsTauTauXaod_EventSelector_H
#define HiggsTauTauXaod_EventSelector_H

#include <EventLoop/Algorithm.h>
#include <TH1F.h>

#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "HiggsTauTauXaod/EventVariablesTool.h"

#include "GoodRunsLists/GoodRunsListSelectionTool.h"

namespace Trig {
  class TrigDecisionTool;
}


class EventSelector : public EL::Algorithm
{


  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
 public:
  // float cutValue;
  std::string trigger_name;

 private:
  GoodRunsListSelectionTool *m_grl; //!
  Trig::TrigDecisionTool *m_trigDecTool; //!
  EventVariablesTool *m_var_tool; //!

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:

  TH1F* m_htaus; //!
  TH1F* m_hcutflow; //!
  // Tree *myTree; //!
  // TH1 *myHist; //!

 
  // this is a standard constructor
  EventSelector ();

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
  ClassDef(EventSelector, 1);
};

#endif
