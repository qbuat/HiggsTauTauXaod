#ifndef HiggsTauTauXaod_UTILS_H
#define HiggsTauTauXaod_UTILS_H

#include "AsgTools/StatusCode.h"

// XAOD ROOT ACCESS
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

#include "xAODBase/IParticle.h"
#include "AthContainers/DataVector.h"
#include "TLorentzVector.h"


namespace Utils {


  template <typename T>
    StatusCode retrieve(T*& cont, std::string name, xAOD::TEvent* event, xAOD::TStore* store)
    {
      if (store != NULL and store->contains< T >(name)) {
	if (store->retrieve(cont, name).isSuccess())
	  return StatusCode::SUCCESS;
	else
	  return StatusCode::FAILURE;
      } else if(event != NULL and event->contains< T >(name)) {
	if (event->retrieve(cont, name).isSuccess())
	  return StatusCode::SUCCESS;
	else
	  return StatusCode::FAILURE;
      } else {
	return StatusCode::FAILURE;
      }
    }


  
  template <typename T>
  const  T* match(const TLorentzVector & p4, const DataVector<T> * cont)
    {
      const T* matched = NULL;
      double dr = 1e6;
      for (const auto part: *cont) {
  	double dr_tmp = p4.DeltaR(part->p4());
  	if (dr_tmp < 0.4 and dr_tmp < dr) {
  	  dr = dr_tmp;
  	  matched = part;
  	}
      }
      return matched;
    }

  bool comparePt(const xAOD::IParticle* t1, const xAOD::IParticle* t2);
 /* { */
 /*    return (t1->pt() > t2->pt() ? true: false); */
 /*  } */

  
}

#endif
