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
  m_hcutflow->GetXaxis()->SetBinLabel(4, "ntaus");
  m_hcutflow->GetXaxis()->SetBinLabel(5, "taus_pt");
  m_hcutflow->GetXaxis()->SetBinLabel(6, "met");
  m_hcutflow->GetXaxis()->SetBinLabel(7, "met_centrality");
  m_hcutflow->GetXaxis()->SetBinLabel(8, "deta_tautau");
  m_hcutflow->GetXaxis()->SetBinLabel(9, "dr_tautau");
  m_hcutflow->GetXaxis()->SetBinLabel(10, "opposite_sign");

  wk()->addOutput(m_hcutflow);


  m_hmet_diff = new TH1F("hmet_diff", "met_diff", 50, -50, 50);
  wk()->addOutput(m_hmet_diff);

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

  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();

  if (asg::ToolStore::contains<EventVariablesTool>("EventVariablesTool")) {
    m_var_tool = asg::ToolStore::get<EventVariablesTool>("EventVariablesTool");
  } else {
    m_var_tool = new EventVariablesTool("EventVariablesTool");
    EL_RETURN_CHECK("initialize", m_var_tool->initialize());
  }

  if (asg::ToolStore::contains<MissingMassTool>("MissingMassTool")) {
    m_mmc_tool = asg::ToolStore::get<MissingMassTool>("MissingMassTool");
  } else {
    m_mmc_tool = new MissingMassTool("MissingMassTool");
    EL_RETURN_CHECK("initialize", m_mmc_tool->setProperty("Decorate", true));
    EL_RETURN_CHECK("initialize", m_mmc_tool->setProperty("CalibSet", "2015HIGHMASS"));
    EL_RETURN_CHECK("initialize", m_mmc_tool->initialize());
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

  ATH_MSG_INFO("Initialization completed");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: execute ()
{
  xAOD::TEvent* event = wk()->xaodEvent();
  xAOD::TStore* store = wk()->xaodStore();

  m_hcutflow->Fill("processed", 1);
  if ((wk()->treeEntry() % 200) == 0)
    ATH_MSG_DEBUG("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());

  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(ei, "EventInfo", event, store));

  // Apply the grl
  if (not ei->eventType(xAOD::EventInfo::IS_SIMULATION)) {
    if (not m_grl->passRunLB(*ei)) {
      wk()->skipEvent();
      return EL::StatusCode::SUCCESS;
    }
  }
  m_hcutflow->Fill("grl", 1);

  // Apply trigger
  bool is_trigger_passed = false;
  for (auto trig: trigger_names) {
    if (m_trigDecTool->isPassed(trig)) {
      is_trigger_passed = true;
      break;
    }
  }
  if (not is_trigger_passed){      
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }
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

  const xAOD::MissingETContainer * mets_ref = 0;
  EL_RETURN_CHECK("execute", Utils::retrieve(mets_ref, "MET_Reference_AntiKt4LCTopo", event, store));
  const xAOD::MissingET * met_ref = *mets_ref->find("FinalClus");

  // ATH_MSG_INFO("ref met = " << met_ref->met() << ", new met = " << met->met());
  m_hmet_diff->Fill( (met->met() - met_ref->met()) / 1e3 );
  if (truth_taus->size() !=2) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }
  m_hcutflow->Fill("ntruthtaus", 1);
    

  if (taus->size() != 2) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }

  m_hcutflow->Fill("ntaus", 1);

  // tau pt cuts
  if (taus->at(0)->pt() < 40000.) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }
  
  if (taus->at(1)->pt() < 30000.) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }

  m_hcutflow->Fill("taus_pt", 1);



  // missing Et
  if (met->met() < 20000.) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }

  m_hcutflow->Fill("met", 1);

  // Compute the event level variables
  EL_RETURN_CHECK("execute", m_var_tool->execute(ei, taus->at(0), taus->at(1), jets, met));


  // met centrality
  bool pass_met_centrality = false;
  if (std::min(std::fabs(ei->auxdata<double>("delta_phi_tau1_met")), 
	       std::fabs(ei->auxdata<double>("delta_phi_tau2_met"))) < TMath::Pi() / 4.
      or ei->auxdata<double>("met_centrality") >= 1)
    pass_met_centrality = true;

  // if (not pass_met_centrality) {
  //   wk()->skipEvent();
  //   return EL::StatusCode::SUCCESS;
  // }  

  m_hcutflow->Fill("met_centrality", 1);


  // deta cut
  if (ei->auxdata<double>("delta_eta") > 1.5) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }

  m_hcutflow->Fill("deta_tautau", 1);

  // DR cut
  if (ei->auxdata<double>("delta_r") < 0.8) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }  

  if (ei->auxdata<double>("delta_r") > 2.4) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }

  m_hcutflow->Fill("dr_tautau", 1);


  if (taus->at(0)->charge() * taus->at(1)->charge() >= 0) {
    wk()->skipEvent();
    return EL::StatusCode::SUCCESS;
  }

  m_hcutflow->Fill("opposite_sign", 1);


  auto code = m_mmc_tool->apply(*ei, taus->at(0), taus->at(1), met, ei->auxdata<int>("njets30"));
  if (not code == CP::CorrectionCode::Ok)
    return StatusCode::FAILURE;

  ATH_MSG_INFO(Form("N(el) =  %d, N(mu) = %d, N(tau) = %d, N(jets) = %d", 
		    (int)electrons->size(), (int)muons->size(), (int)taus->size(), (int)jets->size()));
  // ATH_MSG_INFO("N(leptons) = "<< (electrons->size() + muons->size() + taus->size()));
 
  // ATH_MSG_INFO("Filling event number " 
  // 	       << (int)m_hcutflow->GetBinContent(m_hcutflow->GetNbinsX() - 2)
  // 	       << "in the output");
  m_book.fill_tau(taus->at(0), taus->at(1));
  m_book.fill_evt(ei);
  m_book.fill_met(met);

  // if (electrons->size() + muons->size() + taus->size() != 2)
  //   ATH_MSG_ERROR("Need exactly two leptons to proceed and got : "<<electrons->size() + muons->size() + taus->size());


  

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: postExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HadHadSelector :: finalize ()
{
  ATH_MSG_INFO("finalize");

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

  if (m_mmc_tool) {
    m_mmc_tool = NULL;
    delete m_mmc_tool;
  }

  return EL::StatusCode::SUCCESS;

}



EL::StatusCode HadHadSelector :: histFinalize ()
{
  return EL::StatusCode::SUCCESS;
}


