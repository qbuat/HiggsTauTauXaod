#ifndef HiggsTauTauXaod_TauSelector_H
#define HiggsTauTauXaod_TauSelector_H

#include <EventLoop/Algorithm.h>

#include "TauAnalysisTools/TauTruthMatchingTool.h"


namespace Trig {
  class TrigDecisionTool;
  class TrigTauMatchingTool;
}


class TauSelector : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;

  std::string trigger_name;

 private:
  // Trigger Tools
  Trig::TrigDecisionTool *m_trigDecTool; //!
  Trig::TrigTauMatchingTool *m_trigTauMatchTool; //!

  TauAnalysisTools::TauTruthMatchingTool *m_t2mt; //!

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:


  // Tree *myTree; //!
  // TH1 *myHist; //!


  // this is a standard constructor
  TauSelector ();

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
  ClassDef(TauSelector, 1);
};

#endif
