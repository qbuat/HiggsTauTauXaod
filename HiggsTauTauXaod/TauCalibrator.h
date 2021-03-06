#ifndef HiggsTauTauXaod_TauCalibrator_H
#define HiggsTauTauXaod_TauCalibrator_H

#include <EventLoop/Algorithm.h>
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"

#include "tauRecTools/MvaTESVariableDecorator.h"
#include "tauRecTools/MvaTESEvaluator.h"

class TauCalibrator : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;



  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
  std::string mva_tes_file;

 private:
  
  TauAnalysisTools::TauSmearingTool *m_tausmear;//!
  TauAnalysisTools::TauTruthMatchingTool *m_t2mt; //!
  MvaTESVariableDecorator *m_mva_tes_decor; //!
  MvaTESEvaluator *m_mva_tes_eval; //!

public:
  // Tree *myTree; //!
  // TH1 *myHist; //!



  // this is a standard constructor
  TauCalibrator ();

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
  ClassDef(TauCalibrator, 1);
};

#endif
