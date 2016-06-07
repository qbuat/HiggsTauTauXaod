#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/MetCalculator.h>


// xAOD ROOT ACCESS 
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// tools
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"
#include "METUtilities/CutsMETMaker.h"

// EDM
#include "xAODEventInfo/EventInfo.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODMissingET/MissingET.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "HiggsTauTauXaod/tools.h"
#include "HiggsTauTauXaod/Utils.h"


// this is needed to distribute the algorithm to the workers
ClassImp(MetCalculator)



MetCalculator :: MetCalculator () : m_metmaker(nullptr)
{
}



EL::StatusCode MetCalculator :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("TauCalibrator"));
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MetCalculator :: histInitialize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MetCalculator :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MetCalculator :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MetCalculator :: initialize ()
{

  if (asg::ToolStore::contains<met::METMaker>("METMaker")) {
      m_metmaker = asg::ToolStore::get<met::METMaker>("METMaker");
    } {
      m_metmaker = new met::METMaker("METMaker");
      EL_RETURN_CHECK("initialize", m_metmaker->initialize());
    }  

  ATH_MSG_INFO("Initialization completed");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MetCalculator :: execute ()
{

  ATH_MSG_DEBUG("---------------");
  ATH_MSG_DEBUG("execute next event: " << wk()->treeEntry());
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();


  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", event, store));

  const xAOD::TauJetContainer* taus = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(taus, "CalibratedTaus", event, store));

  const xAOD::JetContainer* jets = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(jets, "CalibratedJets", event, store));

  const xAOD::MuonContainer* muons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(muons, "Muons", event, store));

  const xAOD::ElectronContainer* electrons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(electrons, "Electrons", event, store));

  const xAOD::PhotonContainer* photons = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(photons, "Photons", event, store));

  const xAOD::MissingETContainer* core_met = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(core_met, "MET_Core_AntiKt4LCTopo", event, store));

  const xAOD::MissingETAssociationMap* metMap = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(metMap, "METAssoc_AntiKt4LCTopo", event, store));

  xAOD::MissingETContainer* newMet = new xAOD::MissingETContainer();
  EL_RETURN_CHECK("execute", store->record(newMet, "NewMet"));

  xAOD::MissingETAuxContainer* metAuxCont = new xAOD::MissingETAuxContainer();
  EL_RETURN_CHECK("execute", store->record(metAuxCont, "NewMetAux."));
  newMet->setStore(metAuxCont);



  ConstDataVector<xAOD::ElectronContainer> met_electrons(SG::VIEW_ELEMENTS);
  for (const auto el: *electrons) {
    if (CutsMETMaker::accept(el)) {
	met_electrons.push_back(el);
    }
  }

  ConstDataVector<xAOD::PhotonContainer> met_photons(SG::VIEW_ELEMENTS);
  for (const auto ph: *photons) {
    if (CutsMETMaker::accept(ph)) {
      met_photons.push_back(ph);
    }
  }

  ConstDataVector<xAOD::MuonContainer> met_muons(SG::VIEW_ELEMENTS);
  for (const auto muon: *muons) {
    if (CutsMETMaker::accept(muon)) {
      met_muons.push_back(muon);
    }
  }

  ConstDataVector<xAOD::TauJetContainer> met_taus(SG::VIEW_ELEMENTS);
  for (const auto tau: *taus) {
    if (CutsMETMaker::accept(tau)) {
      met_taus.push_back(tau);
    }
  }

  ConstDataVector<xAOD::JetContainer> met_jets(SG::VIEW_ELEMENTS);
  for (const auto jet: *jets) {
    met_jets.push_back(jet);
  }


  // electrons
  if (electrons->size() != 0) {
    EL_RETURN_CHECK("execute", m_metmaker->rebuildMET("RefEle", xAOD::Type::Electron, newMet, met_electrons.asDataVector(), metMap));
  }

  // photons
  if (photons->size() != 0) {
    EL_RETURN_CHECK("execute", m_metmaker->rebuildMET("RefPhoton", xAOD::Type::Photon, newMet, met_photons.asDataVector(), metMap));
  }

  // taus
  if (taus->size() != 0) {
    EL_RETURN_CHECK("execute", m_metmaker->rebuildMET("RefTau", xAOD::Type::Tau, newMet, met_taus.asDataVector(), metMap));
  }

  // muons
  if (muons->size() != 0) {
    EL_RETURN_CHECK("execute", m_metmaker->rebuildMET("RefMuons", xAOD::Type::Muon, newMet, met_muons.asDataVector(), metMap));
  }

  // jets (no jvt yet)
  if (jets->size() != 0) {
    EL_RETURN_CHECK("execute", m_metmaker->rebuildJetMET("RefJet", "SoftClus", newMet, met_jets.asDataVector(), core_met, metMap, false));
  }


  EL_RETURN_CHECK("execute", m_metmaker->buildMETSum("FinalClus", newMet, MissingETBase::Source::LCTopo));
  EL_RETURN_CHECK("execute", m_metmaker->buildMETSum("FinalTrk", newMet, MissingETBase::Source::Track));


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MetCalculator :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MetCalculator :: finalize ()
{
  ATH_MSG_INFO("finalize");
  if (m_metmaker) {
    m_metmaker = NULL;
    delete m_metmaker;
  }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MetCalculator :: histFinalize ()
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
