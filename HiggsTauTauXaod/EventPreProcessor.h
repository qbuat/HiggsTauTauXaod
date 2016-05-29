#ifndef HiggsTauTauXaod_EventPreProcessor_H
#define HiggsTauTauXaod_EventPreProcessor_H

#include <EventLoop/Algorithm.h>
#include <TH1F.h>

#include "GoodRunsLists/GoodRunsListSelectionTool.h"

namespace TrigConf {
  class xAODConfigTool;
}

namespace Trig {
  class TrigDecisionTool;
}


class EventPreProcessor : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;
  std::string grl_file;

  // float cutValue;
  std::vector<std::string> trigger_names;

 private:
  TrigConf::xAODConfigTool*    m_trigConfTool;  //!
  Trig::TrigDecisionTool*      m_trigDecTool;   //!
  GoodRunsListSelectionTool *m_grl; //!
  
  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!
  TH1F* m_hcutflow;//!


  // this is a standard constructor
  EventPreProcessor ();

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
  ClassDef(EventPreProcessor, 1);
};

#endif
