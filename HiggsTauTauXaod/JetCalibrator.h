#ifndef HiggsTauTauXaod_JetCalibrator_H
#define HiggsTauTauXaod_JetCalibrator_H

#include <EventLoop/Algorithm.h>
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetCalibTools/JetCalibrationTool.h"

class JetCalibrator : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;



  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!

  JetCleaningTool *m_jcl_t;
  JetCalibrationTool *m_jca_t;

  // this is a standard constructor
  JetCalibrator ();

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
  ClassDef(JetCalibrator, 1);
};

#endif