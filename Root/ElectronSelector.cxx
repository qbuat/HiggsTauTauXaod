#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/ElectronSelector.h>

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
#include "xAODEgamma/ElectronContainer.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"

// this is needed to distribute the algorithm to the workers
ClassImp(ElectronSelector)



ElectronSelector :: ElectronSelector () : m_t2mt(nullptr)
{
}



EL::StatusCode ElectronSelector :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("ElectronSelector"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode ElectronSelector :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode ElectronSelector :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode ElectronSelector :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode ElectronSelector :: initialize ()
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



EL::StatusCode ElectronSelector :: execute ()
{

  ATH_MSG_DEBUG("---------------");
  ATH_MSG_DEBUG("execute next event: " << wk()->treeEntry());
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();

  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", event, store));

  const xAOD::ElectronContainer* electrons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(electrons, "Electrons", event, store));

  const xAOD::TruthParticleContainer* truths = 0;
  if (ei->eventType(xAOD::EventInfo::IS_SIMULATION))
    EL_RETURN_CHECK("execute", Utils::retrieve(truths, "TruthTaus", event, store));

  // create a new electron container
  xAOD::ElectronContainer* selected_electrons = new xAOD::ElectronContainer();
  xAOD::AuxContainerBase* selected_electrons_aux = new xAOD::AuxContainerBase();
  selected_electrons->setStore(selected_electrons_aux);

  // block to perform a truth based selection 
  // will only work for the higgs signal though
  if (ei->eventType(xAOD::EventInfo::IS_SIMULATION)) {
    for(auto truth: *truths) {
      if (m_t2mt->getNTauDecayParticles(*truth, MC::PID::ELECTRON, true) == 1) {
	const xAOD::Electron* elec = Utils::match(m_t2mt->getTruthTauP4Vis(*truth), electrons);
	if (elec != NULL) {
	  xAOD::Electron* new_elec = new xAOD::Electron();
	  new_elec->makePrivateStore(*elec);
	  selected_electrons->push_back(new_elec);
	}
      }
    }    
  }
  
  else { 
    // The actual electron selector should go here
    for (const auto elec: *electrons) {
      xAOD::Electron * new_elec = new xAOD::Electron();
      new_elec->makePrivateStore(*elec);
      selected_electrons->push_back(new_elec);
    }
  }

  // sort them by pT
  ATH_MSG_DEBUG("Sort the electrons by descending pT");
  selected_electrons->sort(Utils::comparePt);

  ATH_MSG_DEBUG("Store the selected electrons");
  EL_RETURN_CHECK("execute", store->record(selected_electrons, "SelectedElectrons"));
  EL_RETURN_CHECK("execute", store->record(selected_electrons_aux, "SelectedElectronsAux."));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode ElectronSelector :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode ElectronSelector :: finalize ()
{

  ATH_MSG_INFO("finalize");
  if (m_t2mt) {
    m_t2mt = NULL;
    delete m_t2mt;
  }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode ElectronSelector :: histFinalize ()
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
