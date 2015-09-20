#ifndef HiggsTauTauXaod_EventVariablesTool_H
#define HiggsTauTauXaod_EventVariablesTool_H

#include "AsgTools/AsgTool.h"
#include "xAODBase/IParticle.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODMissingET/MissingET.h"

class EventVariablesTool : public asg::AsgTool

{
 public:
  
  EventVariablesTool(const std::string & name);
  virtual ~EventVariablesTool();
  virtual StatusCode initialize();
  virtual StatusCode execute(const xAOD::EventInfo *ei,
			     const xAOD::IParticle * tau1, 
			     const xAOD::IParticle * tau2,
			     const xAOD::JetContainer * jets,
			     const xAOD::MissingET * met);

 private:

  bool m_mmc;


};

#endif
