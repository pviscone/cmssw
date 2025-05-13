#include <ostream>

#include "IOMC/ParticleGuns/interface/FlatRandomPtAndLxyGunProducer.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

#include "CLHEP/Random/RandFlat.h"

using namespace edm;
using namespace std;

FlatRandomPtAndLxyGunProducer::FlatRandomPtAndLxyGunProducer(const ParameterSet& pset) : BaseFlatGunProducer(pset) {
  ParameterSet defpset;
  ParameterSet pgun_params = pset.getParameter<ParameterSet>("PGunParameters");

  fMinPt = pgun_params.getParameter<double>("MinPt");
  fMaxPt = pgun_params.getParameter<double>("MaxPt");
  fMinEta = pgun_params.getParameter<double>("MinEta");
  fMaxEta = pgun_params.getParameter<double>("MaxEta");
  fMinPhi = pgun_params.getParameter<double>("MinPhi");
  fMaxPhi = pgun_params.getParameter<double>("MaxPhi");
  lxyMin_ = pgun_params.getParameter<double>("LxyMin");
  lxyMax_ = pgun_params.getParameter<double>("LxyMax");
  lzMax_ = pgun_params.getParameter<double>("LzMax");

  produces<HepMCProduct>("unsmeared");
  produces<GenEventInfoProduct>();
}

FlatRandomPtAndLxyGunProducer::~FlatRandomPtAndLxyGunProducer() {
  // no need to cleanup GenEvent memory - done in HepMCProduct
}

void FlatRandomPtAndLxyGunProducer::produce(Event& e, const EventSetup& es) {
  edm::Service<edm::RandomNumberGenerator> rng;
  CLHEP::HepRandomEngine* engine = &rng->getEngine(e.streamID());

  if (fVerbosity > 0) {
    cout << " FlatRandomPtAndLxyGunProducer : Begin New Event Generation" << endl;
  }
  // event loop (well, another step in it...)

  // no need to clean up GenEvent memory - done in HepMCProduct
  //

  // here re-create fEvt (memory)
  //
  fEvt = new HepMC::GenEvent();

  // now actualy, cook up the event from PDGTable and gun parameters
  int barcode = 1;
  for (unsigned int ip = 0; ip < fPartIDs.size(); ++ip) {
    double phi = CLHEP::RandFlat::shoot(engine, fMinPhi, fMaxPhi);
    double eta = CLHEP::RandFlat::shoot(engine, fMinEta, fMaxEta);

    double pt = CLHEP::RandFlat::shoot(engine, fMinPt, fMaxPt);
    double px = pt * cos(phi);
    double py = pt * sin(phi);
    double pz = pt * sinh(eta);

    double lxy = CLHEP::RandFlat::shoot(engine, lxyMin_, lxyMax_);
    double vx = CLHEP::RandFlat::shoot(engine, -lxy, lxy);
    double vy = sqrt(lxy*lxy-vx*vx);
    double vz = CLHEP::RandFlat::shoot(engine, -lzMax_, lzMax_);
    float time = sqrt(vx * vx + vy * vy + vz * vz);

    HepMC::GenVertex* Vtx1 = new HepMC::GenVertex(HepMC::FourVector(vx, vy, vz, time));

    int PartID = fPartIDs[ip];
    const HepPDT::ParticleData* PData = fPDGTable->particle(HepPDT::ParticleID(abs(PartID)));
    double mass = PData->mass().value();
    double energy2 = px * px + py * py + pz * pz + mass * mass;
    double energy = sqrt(energy2);
    HepMC::FourVector p(px, py, pz, energy);
    HepMC::GenParticle* Part = new HepMC::GenParticle(p, PartID, 1);
    Part->suggest_barcode(barcode);
    barcode++;
    Vtx1->add_particle_out(Part);
    fEvt->add_vertex(Vtx1);

    if (fAddAntiParticle) {
      HepMC::GenVertex* Vtx2 = new HepMC::GenVertex(HepMC::FourVector(vx, vy, vz, time));
      HepMC::FourVector ap(-px, -py, -pz, energy);
      int APartID = -PartID;
      if (PartID == 22 || PartID == 23) {
        APartID = PartID;
      }
      HepMC::GenParticle* APart = new HepMC::GenParticle(ap, APartID, 1);
      APart->suggest_barcode(barcode);
      barcode++;
      Vtx2->add_particle_out(APart);
      fEvt->add_vertex(Vtx2);
    }
  }
  fEvt->set_event_number(e.id().event());
  fEvt->set_signal_process_id(20);

  if (fVerbosity > 0) {
    fEvt->print();
  }

  unique_ptr<HepMCProduct> BProduct(new HepMCProduct());
  BProduct->addHepMCData(fEvt);
  e.put(std::move(BProduct), "unsmeared");

  unique_ptr<GenEventInfoProduct> genEventInfo(new GenEventInfoProduct(fEvt));
  e.put(std::move(genEventInfo));

  if (fVerbosity > 0) {
    cout << " FlatRandomPtAndLxyGunProducer : End New Event Generation" << endl;
    fEvt->print();
  }
}

//DEFINE_FWK_MODULE(FlatRandomPtAndLxyGunProducer);

