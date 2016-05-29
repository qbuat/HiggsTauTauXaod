#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <HiggsTauTauXaod/HadHadSelector.h>
// #include "TruthUtils/PIDCodes.h"

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
ClassImp(HadHadSelector)



HadHadSelector :: HadHadSelector () : m_grl(nullptr),
  m_trigDecTool(nullptr),
  m_var_tool(nullptr),
  m_book("Htt_book")
{

}



EL::StatusCode HadHadSelector :: setupJob (EL::Job& job)
{

  job.useXAOD ();
  EL_RETURN_CHECK("setupJob", xAOD::Init("HadHadSelector"));

  // EL::OutputStream out("hist", "xAOD");
  // job.outputAdd(out);

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: histInitialize ()
{

  m_hcutflow = new TH1F("cutflow", "cutflow", 10, 0, 10);
  m_hcutflow->GetXaxis()->SetBinLabel(1, "processed");
  m_hcutflow->GetXaxis()->SetBinLabel(2, "grl");
  m_hcutflow->GetXaxis()->SetBinLabel(3, "trigger");
  m_hcutflow->GetXaxis()->SetBinLabel(4, "ntruthtaus");
  m_hcutflow->GetXaxis()->SetBinLabel(5, "ntaus");
  m_hcutflow->GetXaxis()->SetBinLabel(6, "taus_pt");
  m_hcutflow->GetXaxis()->SetBinLabel(7, "met");
  m_hcutflow->GetXaxis()->SetBinLabel(8, "met_centrality");
  m_hcutflow->GetXaxis()->SetBinLabel(9, "deta_tautau");
  m_hcutflow->GetXaxis()->SetBinLabel(10, "dr_tautau");

  wk()->addOutput(m_hcutflow);

  m_book.book();
  m_book.record(wk());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: initialize ()
{

  store = wk()->xaodStore();
  event = wk()->xaodEvent();

  if (asg::ToolStore::contains<EventVariablesTool>("EventVariablesTool")) {
    m_var_tool = asg::ToolStore::get<EventVariablesTool>("EventVariablesTool");
  } else {
    m_var_tool = new EventVariablesTool("EventVariablesTool");
    EL_RETURN_CHECK("initialize", m_var_tool->initialize());
  }

  if (asg::ToolStore::contains<Trig::TrigDecisionTool>("TrigDecisionTool")) {
    m_trigDecTool = asg::ToolStore::get<Trig::TrigDecisionTool>("TrigDecisionTool");
  } else {
    ATH_MSG_ERROR("The TrigDecisionTool needs to be initialized");
    return EL::StatusCode::FAILURE;
  }

  if (asg::ToolStore::contains<GoodRunsListSelectionTool>("GoodRunsListSelectionTool")) {
    m_grl = asg::ToolStore::get<GoodRunsListSelectionTool>("GoodRunsListSelectionTool");
  } else {
    ATH_MSG_ERROR("The GRL tool needs to be initialized");
    return EL::StatusCode::FAILURE;
  }

  // TFile *file = wk()->getOutputFile ("hist");
  // event = wk()->xaodEvent();
  // EL_RETURN_CHECK("initialize", event->writeTo(file));

  ATH_MSG_INFO("Initialization completed");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: execute ()
{
  // event = wk()->xaodEvent();

  m_hcutflow->Fill("processed", 1);
  if ((wk()->treeEntry() % 200) == 0)
    ATH_MSG_INFO("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());

  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", event, store));

  // Apply the grl
  if (not ei->eventType(xAOD::EventInfo::IS_SIMULATION))
      if (not m_grl->passRunLB(*ei))
	return EL::StatusCode::SUCCESS;

  m_hcutflow->Fill("grl", 1);

  // Apply trigger
  bool is_trigger_passed = false;
  for (auto trig: trigger_names) {
    if (m_trigDecTool->isPassed(trig)) {
      is_trigger_passed = true;
      break;
    }
  }
  if (not is_trigger_passed)
    return EL::StatusCode::SUCCESS;

  // if (not m_trigDecTool->isPassed(trigger_name))
  //   return EL::StatusCode::SUCCESS;

  m_hcutflow->Fill("trigger", 1);


  // Retrieve the containers we may need
  const xAOD::TruthParticleContainer* truth_taus = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(truth_taus, "TruthTaus", event, store));

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
  EL_RETURN_CHECK("execute", Utils::retrieve(mets, "NewMet", event, store));
  const xAOD::MissingET * met = *mets->find("FinalClus");

  if (truth_taus->size() !=2) 
    return EL::StatusCode::SUCCESS;
  m_hcutflow->Fill("ntruthtaus", 1);
    

  if (taus->size() != 2)
    return EL::StatusCode::SUCCESS;
  m_hcutflow->Fill("ntaus", 1);

  // tau pt cuts
  if (taus->at(0)->pt() < 40000.)
    return EL::StatusCode::SUCCESS;
  
  if (taus->at(1)->pt() < 30000.)
    return EL::StatusCode::SUCCESS;

  m_hcutflow->Fill("taus_pt", 1);



  // // missing Et
  // if (met->met() > 20000.)
  //   return EL::StatusCode::SUCCESS;

  m_hcutflow->Fill("met", 1);

  // Compute the event level variables
  EL_RETURN_CHECK("execute", m_var_tool->execute(ei, taus->at(0), taus->at(1), jets, met));


  // met centrality
  if (std::min(std::fabs(ei->auxdata<double>("delta_phi_tau1_met")), 
	       std::fabs(ei->auxdata<double>("delta_phi_tau2_met"))) < TMath::Pi() / 4.
      or ei->auxdata<double>("met_centrality") >= 1)
    return EL::StatusCode::SUCCESS;
  
  m_hcutflow->Fill("met_centrality", 1);


  // deta cut
  if (ei->auxdata<double>("delta_eta") > 1.5)
    return EL::StatusCode::SUCCESS;

  m_hcutflow->Fill("deta_tautau", 1);

  // DR cut
  if (ei->auxdata<double>("delta_r") < 0.4)
    return EL::StatusCode::SUCCESS;
  
  if (ei->auxdata<double>("delta_r") > 2.4)
    return EL::StatusCode::SUCCESS;

  m_hcutflow->Fill("dr_tautau", 1);


  m_book.fill_tau(taus->at(0), taus->at(1));
  m_book.fill_evt(ei);
  m_book.fill_met(met);

  ATH_MSG_DEBUG(Form("N(el) =  %d, N(mu) = %d, N(tau) = %d", (int)electrons->size(), (int)muons->size(), (int)taus->size()));
  ATH_MSG_DEBUG("N(leptons) = "<< (electrons->size() + muons->size() + taus->size()));
  // if (electrons->size() + muons->size() + taus->size() != 2)
  //   ATH_MSG_ERROR("Need exactly two leptons to proceed and got : "<<electrons->size() + muons->size() + taus->size());


  // EL_RETURN_CHECK("execute", event->copy("EventInfo"));
  // ATH_MSG_INFO("fill event "<< wk()->treeEntry());
  // event->fill();
  

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: postExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: finalize ()
{
  ATH_MSG_INFO("finalize");

  // TFile *file = wk()->getOutputFile ("hist");
  // xAOD::TEvent* event = wk()->xaodEvent();
  // EL_RETURN_CHECK("finalize", event->finishWritingTo( file ));
  if (m_grl) {
    m_grl = NULL;
    delete m_grl;
  }

  if (m_trigDecTool) {
    m_trigDecTool = NULL;
    delete m_trigDecTool;
  }

  if (m_var_tool) {
    m_var_tool = NULL;
    delete m_var_tool;
  }
  return EL::StatusCode::SUCCESS;

}



EL::StatusCode HadHadSelector :: histFinalize ()
{
  return EL::StatusCode::SUCCESS;
}


