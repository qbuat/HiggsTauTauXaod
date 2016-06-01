#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/MuonSelector.h>

// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// tools
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"
// #include "TruthUtils/PIDCodes.h"

// EDM
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(MuonSelector)



MuonSelector :: MuonSelector () : m_t2mt(nullptr)
{
}



EL::StatusCode MuonSelector :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("MuonSelector"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MuonSelector :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MuonSelector :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MuonSelector :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MuonSelector :: initialize ()
{

  if (asg::ToolStore::contains<TauAnalysisTools::TauTruthMatchingTool>("TauTruthMatchingTool")) {
    m_t2mt = asg::ToolStore::get<TauAnalysisTools::TauTruthMatchingTool>("TauTruthMatchingTool");
  } else {
    m_t2mt = new TauAnalysisTools::TauTruthMatchingTool("TauTruthMatchingTool");
    EL_RETURN_CHECK("initialize", m_t2mt->initialize());
  }

  ATH_MSG_INFO("Initialization completed");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MuonSelector :: execute ()
{

  ATH_MSG_DEBUG("execute next event: " << wk()->treeEntry());
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();


  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", event, store));

  const xAOD::MuonContainer* muons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(muons, "Muons", event, store));

  const xAOD::TruthParticleContainer* truths = 0;
  if (ei->eventType(xAOD::EventInfo::IS_SIMULATION))
    EL_RETURN_CHECK("execute", Utils::retrieve(truths, "TruthTaus", event, store));

  xAOD::MuonContainer* selected_muons = new xAOD::MuonContainer();
  xAOD::AuxContainerBase* selected_muons_aux = new xAOD::AuxContainerBase();
  selected_muons->setStore(selected_muons_aux);

  // block to perform a truth based selection 
  // will only work for the higgs signal though
  if (ei->eventType(xAOD::EventInfo::IS_SIMULATION)) {
    for(auto truth: *truths) { 
      if (m_t2mt->getNTauDecayParticles(*truth, MC::PID::MUON, true) == 1) {
	const xAOD::Muon* muon = Utils::match(m_t2mt->getTruthTauP4Vis(*truth), muons);
	if (muon != NULL) {
	  xAOD::Muon* new_muon = new xAOD::Muon();
	  new_muon->makePrivateStore(*muon);
	  selected_muons->push_back(new_muon);
	}
      }
    }    
  }

  else {
    for (const auto muon: *muons) {
      xAOD::Muon *new_muon = new xAOD::Muon();
      new_muon->makePrivateStore(*muon);
      selected_muons->push_back(new_muon);
    }
  }

  // sort them by pT
  ATH_MSG_DEBUG("Sort the muon by descending pT");
  selected_muons->sort(Utils::comparePt);

  ATH_MSG_DEBUG("Store the selected muons");
  EL_RETURN_CHECK("execute", store->record(selected_muons, "SelectedMuons"));
  EL_RETURN_CHECK("execute", store->record(selected_muons_aux, "SelectedMuonsAux."));



  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MuonSelector :: postExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MuonSelector :: finalize ()
{
  ATH_MSG_INFO("finalize");
  if (m_t2mt) {
    m_t2mt = NULL;
    delete m_t2mt;
  }
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MuonSelector :: histFinalize ()
{
  return EL::StatusCode::SUCCESS;
}
