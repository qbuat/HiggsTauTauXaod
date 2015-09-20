#include "HiggsTauTauXaod/Utils.h"

bool Utils::comparePt(const xAOD::IParticle* t1, const xAOD::IParticle* t2)
{
    return (t1->pt() > t2->pt() ? true: false);
}


// template <typename T>
// StatusCode Utls::retrieve(T*& cont, std::string name, xAOD::TEvent * event, xAOD::TStore *store)
// {
//   if (store != NULL and store->contains< T >(name))
//     return store->retrieve(cont, name);
//   else if(event != NULL and event->contains< T >(name))
//     return event->retrieve(cont, name);
//   else
//     return StatusCode::FAILURE;
// }


// template <typename T>
// T* Utls::match(const TLorentzVector & p4, const DataVector<T> * cont)
// {
//   T* matched = NULL;
//   double dr = 1e6;
//   for (auto part: *cont) {
//     double dr_tmp = p4.DeltaR(part->p4());
//     if (dr_tmp < 0.4 and dr_tmp < dr) {
//       dr = dr_tmp;
//       matched = part;
//     }
//   }
//   return matched;
// }
