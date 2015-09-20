#include "HiggsTauTauXaod/EventVariablesTool.h"


EventVariablesTool::EventVariablesTool(const std::string & name): AsgTool(name)
{
  declareProperty("MMC", m_mmc=false);

}

EventVariablesTool::~EventVariablesTool()
{} 

StatusCode EventVariablesTool::initialize()
{
  return StatusCode::SUCCESS;
}


StatusCode EventVariablesTool::execute(const xAOD::EventInfo * ei, 
				       const xAOD::IParticle * tau1, 
				       const xAOD::IParticle * tau2,
				       const xAOD::JetContainer * jets,
				       const xAOD::MissingET * met)
{
  ATH_MSG_INFO( (tau1->p4() + tau2->p4()).Pt());

  // first tau
  ei->auxdecor<double>("tau1_pt") = tau1->pt();
  ei->auxdecor<double>("tau1_eta") = tau1->eta();
  ei->auxdecor<double>("tau1_phi") = tau1->phi();
  ei->auxdecor<double>("tau1_m") = tau1->m();

  // second tau
  ei->auxdecor<double>("tau1_pt") = tau1->pt();
  ei->auxdecor<double>("tau1_eta") = tau1->eta();
  ei->auxdecor<double>("tau1_phi") = tau1->phi();
  ei->auxdecor<double>("tau1_m") = tau1->m();

  // ditau system
  ei->auxdecor<double>("delta_eta") = fabs(tau1->eta() - tau2->eta());
  ei->auxdecor<double>("delta_phi") = tau1->p4().DeltaPhi(tau2->p4());
  ei->auxdecor<double>("delta_r") = tau1->p4().DeltaR(tau2->p4());
  if (m_mmc)
    ATH_MSG_INFO("store MMC result");

  return StatusCode::SUCCESS;
}
