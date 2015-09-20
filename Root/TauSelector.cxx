#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/TauSelector.h>
#include "TruthUtils/PIDCodes.h"


// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// tools
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(TauSelector)

TauSelector :: TauSelector ()
{
}



EL::StatusCode TauSelector :: setupJob (EL::Job& job)
{

  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("TauSelector"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauSelector :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauSelector :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauSelector :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauSelector :: initialize ()
{
  if (asg::ToolStore::contains<TauAnalysisTools::TauTruthMatchingTool>("TauTruthMatchingTool")) {
    m_t2mt = asg::ToolStore::get<TauAnalysisTools::TauTruthMatchingTool>("TauTruthMatchingTool");
  } else {
    m_t2mt = new TauAnalysisTools::TauTruthMatchingTool("TauTruthMatchingTool");
    EL_RETURN_CHECK("initialize", m_t2mt->initialize());
  }

  xAOD::TEvent* event = wk()->xaodEvent();
  ATH_MSG_INFO("Number of events = " << event->getEntries());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauSelector :: execute ()
{

  ATH_MSG_DEBUG("---------------");
  ATH_MSG_DEBUG("execute next event: " << wk()->treeEntry());
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();


  const xAOD::TauJetContainer* taus = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(taus, "TauJets", event, store));

  const xAOD::TruthParticleContainer* truths = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(truths, "SelectedTruthTaus", event, store));


  xAOD::TauJetContainer* selected_taus = new xAOD::TauJetContainer();
  xAOD::AuxContainerBase* selected_taus_aux = new xAOD::AuxContainerBase();
  selected_taus->setStore(selected_taus_aux);

  for(auto truth: *truths) {
    if (truth->isTau() and (bool)truth->auxdata<char>("IsHadronicTau")) {
      const xAOD::TauJet* tau = Utils::match(m_t2mt->getTruthTauP4Vis(*truth), taus);
      if (tau != NULL) {
	xAOD::TauJet* new_tau = new xAOD::TauJet();
	new_tau->makePrivateStore(*tau);
	selected_taus->push_back(new_tau);
      }
    }
  }    


  // sort them by pT
  selected_taus->sort(Utils::comparePt);

  ATH_MSG_DEBUG("Store the selected taus");
  EL_RETURN_CHECK("execute", store->record(selected_taus, "SelectedTaus"));
  EL_RETURN_CHECK("execute", store->record(selected_taus_aux, "SelectedTausAux."));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauSelector :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauSelector :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauSelector :: histFinalize ()
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
