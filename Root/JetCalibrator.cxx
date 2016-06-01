#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/JetCalibrator.h>

// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// tools
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM
#include "xAODBase/IParticleHelpers.h"
#include "xAODJet/JetContainer.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(JetCalibrator)



JetCalibrator :: JetCalibrator () : m_jca_t(nullptr)
{
}



EL::StatusCode JetCalibrator :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("JetCalibrator"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: histInitialize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: initialize ()
{

  if (asg::ToolStore::contains<JetCalibrationTool>("JetCalibrationTool")) {
    m_jca_t = asg::ToolStore::get<JetCalibrationTool>("JetCalibrationTool");
  } else {
    m_jca_t = new JetCalibrationTool("JetCalibrationTool",
				     "AntiKt4LCTopo",
				     "JES_MC15Prerecommendation_April2015.config",
				     "JetArea_Residual_Origin_EtaJES_GSC",
				     false);
    EL_RETURN_CHECK("initialize", m_jca_t->initializeTool("JetCalibrationTool"));
  }
      

  ATH_MSG_INFO("Initialization completed");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: execute ()
{
  ATH_MSG_DEBUG("---------------");
  ATH_MSG_DEBUG("execute next event: " << wk()->treeEntry());
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();

  const xAOD::JetContainer* jets = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(jets, "AntiKt4LCTopoJets", event, store));

  // create output container with selected jets
  xAOD::JetContainer* calib_jets = new xAOD::JetContainer();
  xAOD::AuxContainerBase* calib_jets_aux = new xAOD::AuxContainerBase();
  calib_jets->setStore(calib_jets_aux);

  for (const auto jet: *jets) {
    xAOD::Jet* new_jet = 0; //new xAOD::Jet();
    m_jca_t->calibratedCopy(*jet, new_jet);
    // new_jet->makePrivateStore(*jet);
    
    // m_jca_t->applyCorrection(*new_jet);
    calib_jets->push_back(new_jet);
    if (not xAOD::setOriginalObjectLink(*jet, *new_jet)) {
      ATH_MSG_ERROR("Fail to set the links");
      return EL::StatusCode::FAILURE;
    }
  }
  
  
  ATH_MSG_DEBUG("Store the calibrated jets");
  EL_RETURN_CHECK("execute", store->record(calib_jets, "CalibratedJets"));
  EL_RETURN_CHECK("execute", store->record(calib_jets_aux, "CalibratedJetsAux."));


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: finalize ()
{

  ATH_MSG_INFO("finalize");
  if (m_jca_t) {
    m_jca_t = NULL;
    delete m_jca_t;
  }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: histFinalize ()
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
