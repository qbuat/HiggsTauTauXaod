#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/TauCalibrator.h>


// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// tools
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM
#include "xAODBase/IParticleHelpers.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"


// this is needed to distribute the algorithm to the workers
ClassImp(TauCalibrator)



TauCalibrator :: TauCalibrator () : m_tausmear(nullptr),
  m_mva_tes_decor(nullptr),
  m_mva_tes_eval(nullptr)
{

}



EL::StatusCode TauCalibrator :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("TauCalibrator"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauCalibrator :: histInitialize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauCalibrator :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauCalibrator :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauCalibrator :: initialize ()
{
  if (asg::ToolStore::contains<TauAnalysisTools::TauSmearingTool>("TauSmearingTool")){
      m_tausmear = asg::ToolStore::get<TauAnalysisTools::TauSmearingTool>("TauSmearingTool");
  } else {
    m_tausmear = new TauAnalysisTools::TauSmearingTool("TauSmearingTool");
    EL_RETURN_CHECK("initialize", m_tausmear->initialize());
  }

  if (asg::ToolStore::contains<TauAnalysisTools::TauTruthMatchingTool>("TauTruthMatchingTool")) {
    m_t2mt = asg::ToolStore::get<TauAnalysisTools::TauTruthMatchingTool>("TauTruthMatchingTool");
  } else {
    ATH_MSG_FATAL("TauTruthMatching should be initialized earlier!");
    return EL::StatusCode::FAILURE;
  }

  if (asg::ToolStore::contains<MvaTESVariableDecorator>("MvaTESVariableDecorator")) {
    m_mva_tes_decor = asg::ToolStore::get<MvaTESVariableDecorator>("MvaTESVariableDecorator");
  } else {
    m_mva_tes_decor = new MvaTESVariableDecorator("MvaTESVariableDecorator");
    EL_RETURN_CHECK("initialize", m_mva_tes_decor->initialize());
  }

  if (asg::ToolStore::contains<MvaTESEvaluator>("MvaTESEvaluator")) {
    m_mva_tes_eval= asg::ToolStore::get<MvaTESEvaluator>("MvaTESEvaluator");
  } else {
    m_mva_tes_eval = new MvaTESEvaluator("MvaTESEvaluator");
    EL_RETURN_CHECK("initialize", m_mva_tes_eval->setProperty("WeightFileName", mva_tes_file));
    EL_RETURN_CHECK("initialize", m_mva_tes_eval->initialize());
  }



  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauCalibrator :: execute ()
{
  ATH_MSG_DEBUG("---------------");
  ATH_MSG_DEBUG("execute next event: " << wk()->treeEntry());
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();


  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", event, store));

  const xAOD::TauJetContainer* taus = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(taus, "TauJets", event, store));

  const xAOD::TruthParticleContainer* truthtaus = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(truthtaus, "TruthTaus", event, store));

  xAOD::TauJetContainer* calibrated_taus = new xAOD::TauJetContainer();
  xAOD::AuxContainerBase* calibrated_taus_aux = new xAOD::AuxContainerBase();
  calibrated_taus->setStore(calibrated_taus_aux);

  EL_RETURN_CHECK("execute", m_mva_tes_decor->eventInitialize());
  EL_RETURN_CHECK("execute", m_mva_tes_eval->eventInitialize());

  for (const auto tau: *taus) {
    m_t2mt->getTruth(*tau);
    xAOD::TauJet* new_tau = new xAOD::TauJet();
    new_tau->makePrivateStore(*tau);
    // TO BE FIXED 
    auto code = m_tausmear->applyCorrection(*new_tau);
    if (not code == CP::CorrectionCode::Ok) {
      ATH_MSG_FATAL("Tau smearing failed miserably");
      return EL::StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("old calib pt = " << new_tau->pt());
    EL_RETURN_CHECK("execute", m_mva_tes_decor->execute(*new_tau));
    EL_RETURN_CHECK("execute", m_mva_tes_eval->execute(*new_tau));
    new_tau->setP4(
		   new_tau->auxdata<float>("ptFinalCalib"),
		   new_tau->etaPanTauCellBased(),
		   new_tau->phiPanTauCellBased(), 0);
    ATH_MSG_DEBUG("new calib pt = " << new_tau->pt());
    ATH_MSG_DEBUG(new_tau->auxdata<float>("ptFinalCalib"));
    calibrated_taus->push_back(new_tau);
    if (not xAOD::setOriginalObjectLink(*tau, *new_tau)) {
      ATH_MSG_ERROR("Fail to set the links");
      return EL::StatusCode::FAILURE;
    }
  }

  ATH_MSG_DEBUG("Store the selected taus");
  EL_RETURN_CHECK("execute", store->record(calibrated_taus, "CalibratedTaus"));
  EL_RETURN_CHECK("execute", store->record(calibrated_taus_aux, "CalibratedTausAux."));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauCalibrator :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauCalibrator :: finalize ()
{
  if (m_tausmear) {
    m_tausmear = NULL;
    delete m_tausmear;
  }
  if (m_t2mt) {
    m_t2mt = NULL;
    delete m_t2mt;
  }

  if (m_mva_tes_decor) {
    m_mva_tes_decor = NULL;
    delete m_mva_tes_decor;
  }

  if (m_mva_tes_eval) {
    m_mva_tes_eval = NULL;
    delete m_mva_tes_eval;
  }


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauCalibrator :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.
  return EL::StatusCode::SUCCESS;
}
