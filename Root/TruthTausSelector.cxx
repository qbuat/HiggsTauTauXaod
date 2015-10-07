#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/TruthTausSelector.h>

// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// tools
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(TruthTausSelector)



TruthTausSelector :: TruthTausSelector ()
{
}



EL::StatusCode TruthTausSelector :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("TruthTausSelector"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TruthTausSelector :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TruthTausSelector :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TruthTausSelector :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TruthTausSelector :: initialize ()
{
  if (asg::ToolStore::contains<TauAnalysisTools::TauTruthMatchingTool>("TauTruthMatchingTool")) {
    m_t2mt = asg::ToolStore::get<TauAnalysisTools::TauTruthMatchingTool>("TauTruthMatchingTool");
  } else {
    m_t2mt = new TauAnalysisTools::TauTruthMatchingTool("TauTruthMatchingTool");
    EL_RETURN_CHECK("initialize", m_t2mt->initialize());
  }

  xAOD::TEvent* event = wk()->xaodEvent();

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TruthTausSelector :: execute ()
{

  ATH_MSG_DEBUG("---------------");
  ATH_MSG_DEBUG("execute next event: " << wk()->treeEntry());
  // odly enough, we don't need a direct instance of the event here
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();


  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", event, store));

  if (not ei->eventType(xAOD::EventInfo::IS_SIMULATION))
    return EL::StatusCode::SUCCESS;

  // event initialization of the tools (if needed)
  EL_RETURN_CHECK("execute", m_t2mt->initializeEvent());

  xAOD::TruthParticleContainer* xTruthTauContainer = m_t2mt->getTruthTauContainer();

  if (xTruthTauContainer->size() < 2) {
    ATH_MSG_WARNING("Something is wrong with the truth matching !");
    for (auto p: *xTruthTauContainer) {
      ATH_MSG_WARNING(Form("Idx = %d, pT = %f, eta = %f, phi = %f, pdgId = %d, status = %d", 
			   (int)p->index(), p->pt(), p->eta(), p->phi(), p->pdgId(), p->status()));
    }
    return EL::StatusCode::SUCCESS;
  }

  
  // sort the truth taus and select only the first 2
  // TODO: to be generalized 
  xTruthTauContainer->sort(Utils::comparePt);
  xAOD::TruthParticle* truth1 = xTruthTauContainer->at(0);
  xAOD::TruthParticle* truth2 = xTruthTauContainer->at(1);
  
  xAOD::TruthParticleContainer* selected_truth = new xAOD::TruthParticleContainer() ;
  xAOD::AuxContainerBase* selected_truth_aux = new xAOD::AuxContainerBase();
  selected_truth->setStore(selected_truth_aux);

  xAOD::TruthParticle* new_truth1 = new xAOD::TruthParticle();
  new_truth1->makePrivateStore(*truth1);
  selected_truth->push_back(new_truth1);

  xAOD::TruthParticle* new_truth2 = new xAOD::TruthParticle();
  new_truth2->makePrivateStore(*truth2);
  selected_truth->push_back(new_truth2);

  ATH_MSG_DEBUG("Store the selected truth taus");
  EL_RETURN_CHECK("execute", store->record(selected_truth, "SelectedTruthTaus"));
  EL_RETURN_CHECK("execute", store->record(selected_truth_aux, "SelectedTruthTausAux."));


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TruthTausSelector :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TruthTausSelector :: finalize ()
{
  ATH_MSG_INFO("finalize");
  if (m_t2mt) {
    m_t2mt = NULL;
    delete m_t2mt;
  }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TruthTausSelector :: histFinalize ()
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
