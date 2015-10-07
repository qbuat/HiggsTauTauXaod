#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/EventPreProcessor.h>
#include <TSystem.h>

// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// EDM
#include "xAODEventInfo/EventInfo.h"

// tools
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(EventPreProcessor)



EventPreProcessor :: EventPreProcessor () : m_trigConfTool(nullptr),
  m_trigDecTool(nullptr),
  m_grl(nullptr)
{
}



EL::StatusCode EventPreProcessor :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("EventSelector"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventPreProcessor :: histInitialize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventPreProcessor :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventPreProcessor :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventPreProcessor :: initialize ()
{

  // Trigger Config tool
  m_trigConfTool = new TrigConf::xAODConfigTool("xAODConfigTool");
  EL_RETURN_CHECK("initialize", m_trigConfTool->initialize());
  ToolHandle< TrigConf::ITrigConfigTool > configHandle(m_trigConfTool);

  // Trigger decision tool
  m_trigDecTool = new Trig::TrigDecisionTool( "TrigDecisionTool" );
  EL_RETURN_CHECK("initialize", m_trigDecTool->setProperty("ConfigTool", configHandle));
  EL_RETURN_CHECK("initialize", m_trigDecTool->setProperty("TrigDecisionKey", "xTrigDecision"));
  EL_RETURN_CHECK("initialize", m_trigDecTool->setProperty("OutputLevel", MSG::ERROR));
  EL_RETURN_CHECK("initialize", m_trigDecTool->initialize());

  // GRL tool
  m_grl = new GoodRunsListSelectionTool("GoodRunsListSelectionTool");
  const char* GRLFilePath = grl_file.c_str();
  const char* fullGRLFilePath = gSystem->ExpandPathName (GRLFilePath);
  std::vector<std::string> vecStringGRL;
  vecStringGRL.push_back(fullGRLFilePath);
  EL_RETURN_CHECK("initialize",m_grl->setProperty( "GoodRunsListVec", vecStringGRL));
  EL_RETURN_CHECK("initialize",m_grl->setProperty("PassThrough", false));
  EL_RETURN_CHECK("initialize",m_grl->initialize());

  ATH_MSG_INFO("Initialization completed");

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventPreProcessor :: execute ()
{
  ATH_MSG_DEBUG("==========================================");
  ATH_MSG_DEBUG("execute next event: "<< wk()->treeEntry());

  xAOD::TEvent* event = wk()->xaodEvent();
  // xAOD::TStore* store = wk()->xaodStore();

  if ((wk()->treeEntry() % 200) == 0)
    ATH_MSG_INFO("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventPreProcessor :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventPreProcessor :: finalize ()
{
  ATH_MSG_INFO("finalize");
  if (m_trigConfTool) {
    m_trigConfTool = NULL;
    delete m_trigConfTool;
  }

  if (m_trigDecTool) {
    m_trigDecTool = NULL;
    delete m_trigDecTool;
  }

  if (m_grl) {
    m_grl = NULL;
    delete m_grl;
  }
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventPreProcessor :: histFinalize ()
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
