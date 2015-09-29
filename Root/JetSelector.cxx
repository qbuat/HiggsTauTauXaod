#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/JetSelector.h>

// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM
#include "xAODTau/TauJetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(JetSelector)



JetSelector :: JetSelector ()
{

}



EL::StatusCode JetSelector :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("JetSelector"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetSelector :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetSelector :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetSelector :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetSelector :: initialize ()
{
  ATH_MSG_INFO("Initialization completed");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetSelector :: execute ()
{
  ATH_MSG_DEBUG("---------------");
  ATH_MSG_DEBUG("execute next event: " << wk()->treeEntry());
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();

  // retrieve previously stored containers
  const xAOD::JetContainer* jets = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(jets, "AntiKt4LCTopoJets", event, store));

  const xAOD::TauJetContainer* taus = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(taus, "SelectedTaus", event, store));

  const xAOD::MuonContainer* muons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(muons, "SelectedMuons", event, store));

  const xAOD::ElectronContainer* electrons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(electrons, "SelectedElectrons", event, store));

  // create output container with selected jets
  xAOD::JetContainer* selected_jets = new xAOD::JetContainer();
  xAOD::AuxContainerBase* selected_jets_aux = new xAOD::AuxContainerBase();
  selected_jets->setStore(selected_jets_aux);

  for (const auto jet: *jets) {
      
    // pt cut
    if (jet->pt() < 30000.) 
      continue;
    
    // eta cut
    if (fabs(jet->eta()) > 4.5)
      continue;

    for (auto tau: *taus)
      if (jet->p4().DeltaR(tau->p4()))
	continue;
    
    for (auto muon: *muons)
      if (jet->p4().DeltaR(muon->p4()))
	continue;

    for (auto electron: *electrons)
      if (jet->p4().DeltaR(electron->p4()))
	continue;

    xAOD::Jet* new_jet = new xAOD::Jet();
    new_jet->makePrivateStore(*jet);
    selected_jets->push_back(new_jet);
  }
  // sort them by pT
  selected_jets->sort(Utils::comparePt);


  ATH_MSG_DEBUG("Store the selected jets");
  EL_RETURN_CHECK("execute", store->record(selected_jets, "SelectedJets"));
  EL_RETURN_CHECK("execute", store->record(selected_jets_aux, "SelectedJetsAux."));

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetSelector :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetSelector :: finalize ()
{
  ATH_MSG_INFO("finalize");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode JetSelector :: histFinalize ()
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
