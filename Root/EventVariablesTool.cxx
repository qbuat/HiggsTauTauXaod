#include "HiggsTauTauXaod/EventVariablesTool.h"
#include "TMatrixT.h"
#include "TVectorT.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"


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
  // ATH_MSG_INFO( (tau1->p4() + tau2->p4()).Pt());

  // first tau
  ei->auxdecor<double>("tau1_pt") = tau1->pt();
  ei->auxdecor<double>("tau1_eta") = tau1->eta();
  ei->auxdecor<double>("tau1_phi") = tau1->phi();
  ei->auxdecor<double>("tau1_m") = tau1->m();

  // second tau
  ei->auxdecor<double>("tau2_pt") = tau2->pt();
  ei->auxdecor<double>("tau2_eta") = tau2->eta();
  ei->auxdecor<double>("tau2_phi") = tau2->phi();
  ei->auxdecor<double>("tau2_m") = tau2->m();

  // ditau system
  ei->auxdecor<double>("delta_eta") = fabs(tau1->eta() - tau2->eta());
  ei->auxdecor<double>("delta_phi") = tau1->p4().DeltaPhi(tau2->p4());
  ei->auxdecor<double>("delta_r") = tau1->p4().DeltaR(tau2->p4());

  // ditau + met
  ei->auxdecor<double>("met_centrality") = METcentrality(tau1, tau2, met);
  ei->auxdecor<double>("delta_phi_tau1_met") = DeltaPhi(tau1, met);
  ei->auxdecor<double>("delta_phi_tau2_met") = DeltaPhi(tau2, met);
  ei->auxdecor<double>("collinear_mass") = CollinearMass(tau1, tau2, met);

  if (m_mmc)
    ATH_MSG_INFO("store MMC result");

  // jets
  ei->auxdecor<int>("njets") = (int)jets->size();

  int njets25 = 0;
  for (const auto* j: *jets) {
    if (j->pt() > 25000.) 
      njets25++;
  }
  ei->auxdecor<int>("njets25") = njets25;

  return StatusCode::SUCCESS;
}


double EventVariablesTool::METcentrality(const xAOD::IParticle *p1, const xAOD::IParticle *p2, const xAOD::MissingET* met){

  double d = std::sin(p2->phi() - p1->phi());
  if (d != 0) {
    double A = std::sin(met->phi() - p1->phi()) / d;
    double B = std::sin(p2->phi() - met->phi()) / d;
    return (A + B) / std::sqrt(A * A + B * B);
  }

  return -1234;
}

double EventVariablesTool::DeltaPhi(const xAOD::IParticle* p, const xAOD::MissingET* met)
{
  double phi1 = p->phi();
  double phi2 = met->phi();
  return TMath::Pi() - std::fabs( std::fabs( phi1 - phi2 ) - TMath::Pi() );
}


double EventVariablesTool::CollinearMass(const xAOD::IParticle * p1, const xAOD::IParticle *p2, const xAOD::MissingET* met) 
{

  TLorentzVector k1 = p1->p4();
  TLorentzVector k2 = p2->p4();

  TMatrixD K(2, 2);
  K(0, 0) = k1.Px();      K(0, 1) = k2.Px();
  K(1, 0) = k1.Py();      K(1, 1) = k2.Py();

  if(K.Determinant()==0)
    return -1234.;

  TMatrixD M(2, 1);
  M(0, 0) = met->mpx();
  M(1, 0) = met->mpy();

  TMatrixD Kinv = K.Invert();

  TMatrixD X(2, 1);
  X = Kinv * M;

  double X1 = X(0, 0);        double X2 = X(1, 0);
  double x1 = 1. / (1. + X1); double x2 = 1. / (1. + X2);

  TLorentzVector part1 = k1 * (1 / x1);
  TLorentzVector part2 = k2 * (1 / x2);

  double m = (part1 + part2).M();
  return m;
}

