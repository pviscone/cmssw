#ifndef FlatRandomPtAndLxyGunProducer_H
#define FlatRandomPtAndLxyGunProducer_H

/** \class FlatRandomPtAndLxyGunProducer


 * Contact Piero Viscone
 ***************************************/

#include "IOMC/ParticleGuns/interface/BaseFlatGunProducer.h"

namespace edm {

  class FlatRandomPtAndLxyGunProducer : public BaseFlatGunProducer {
  public:
    FlatRandomPtAndLxyGunProducer(const ParameterSet& pset);
    ~FlatRandomPtAndLxyGunProducer() override;

    void produce(Event& e, const EventSetup& es) override;

  private:
    // data members

    double fMinPt;
    double fMaxPt;
    double fMinEta;
    double fMaxEta;
    double fMinPhi;
    double fMaxPhi;
    double lxyMin_;
    double lxyMax_;
    double lzMax_;
  };
}  // namespace edm

#endif
