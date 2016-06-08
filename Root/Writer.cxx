#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/Writer.h>

// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// EventLoop stuff
#include "EventLoop/OutputStream.h"

// EDM
#include "xAODEventInfo/EventInfo.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingET.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

// tools
#include "TrigDecisionTool/TrigDecisionTool.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(Writer)



Writer :: Writer ()
{
}



EL::StatusCode Writer :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("Writer"));

  EL::OutputStream out("hist", "xAOD");
  job.outputAdd(out);

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Writer :: histInitialize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Writer :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Writer :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Writer :: initialize ()
{
  m_store = wk()->xaodStore();
  m_event = wk()->xaodEvent();

  TFile *file = wk()->getOutputFile ("hist");
  EL_RETURN_CHECK("initialize", m_event->writeTo(file));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Writer :: execute ()
{
  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", m_event, m_store));

  m_event->copy("EventInfo");
 // if (not m_event->record(ei, "EventInfo"))
 //    return EL::StatusCode::FAILURE;

  ATH_MSG_INFO("fill event "<< wk()->treeEntry());
  m_event->fill();

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Writer :: postExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Writer :: finalize ()
{
  ATH_MSG_INFO("finalize");
  TFile *file = wk()->getOutputFile ("hist");
  EL_RETURN_CHECK("finalize", m_event->finishWritingTo( file ));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Writer :: histFinalize ()
{
  return EL::StatusCode::SUCCESS;
}
