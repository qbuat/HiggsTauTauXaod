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
#include "xAODJet/JetContainer.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(JetCalibrator)



JetCalibrator :: JetCalibrator ()
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
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetCalibrator :: initialize ()
{
  if (asg::ToolStore::contains<JetCleaningTool>("JetCleaningTool")) {
    m_jcl_t = asg::ToolStore::get<JetCleaningTool>("JetCleaningTool");
  } else {
    m_jcl_t = new JetCleaningTool("JetCleaningTool");
    EL_RETURN_CHECK("initialize", m_jcl_t->setProperty("CutLevel", "LooseBad"));
    EL_RETURN_CHECK("initialize", m_jcl_t->setProperty("DoUgly", false));
    EL_RETURN_CHECK("initialize", m_jcl_t->initialize());
  }

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
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.
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

  if (m_jcl_t) {
    m_jcl_t = NULL;
    delete m_jcl_t;
  }

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
