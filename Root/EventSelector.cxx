#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/EventSelector.h>
#include "TruthUtils/PIDCodes.h"

// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// EDM
#include "xAODEventInfo/EventInfo.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

// tools
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(EventSelector)



EventSelector :: EventSelector ()
{

}



EL::StatusCode EventSelector :: setupJob (EL::Job& job)
{

  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("EventSelector"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventSelector :: histInitialize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventSelector :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventSelector :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventSelector :: initialize ()
{

  if (asg::ToolStore::contains<EventVariablesTool>("EventVariablesTool")) {
    m_var_tool = asg::ToolStore::get<EventVariablesTool>("EventVariablesTool");
  } else {
    m_var_tool = new EventVariablesTool("EventVariablesTool");
    EL_RETURN_CHECK("initialize", m_var_tool->initialize());
  }


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventSelector :: execute ()
{
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();

  if ((wk()->treeEntry() % 200) == 0)
    ATH_MSG_INFO("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());

  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", event, store));

  const xAOD::TauJetContainer* taus = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(taus, "SelectedTaus", event, store));

  const xAOD::JetContainer* jets = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(jets, "SelectedJets", event, store));

  const xAOD::MuonContainer* muons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(muons, "SelectedMuons", event, store));

  const xAOD::ElectronContainer* electrons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(electrons, "SelectedElectrons", event, store));

  // TODO: proper MET constructor with the tool
  const xAOD::MissingETContainer * mets = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(mets, "MET_Reference_AntiKt4LCTopo", event, store));


  ATH_MSG_DEBUG(Form("N(el) =  %d, N(mu) = %d, N(tau) = %d", (int)electrons->size(), (int)muons->size(), (int)taus->size()));
  ATH_MSG_DEBUG("N(leptons) = "<< (electrons->size() + muons->size() + taus->size()));
  if (electrons->size() + muons->size() + taus->size() != 2)
    ATH_MSG_ERROR("Need exactly two leptons to proceed and got : "<<electrons->size() + muons->size() + taus->size());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventSelector :: postExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventSelector :: finalize ()
{
  ATH_MSG_INFO("finalize");
  if (m_var_tool) {
    m_var_tool = NULL;
    delete m_var_tool;
  }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode EventSelector :: histFinalize ()
{
  return EL::StatusCode::SUCCESS;
}


