#include "FastSimulation/SimplifiedGeometryPropagator/interface/Decayer.h"
#include "FastSimulation/SimplifiedGeometryPropagator/interface/Particle.h"
#include "FWCore/ServiceRegistry/interface/RandomEngineSentry.h"
#include "GeneratorInterface/Pythia8Interface/interface/P8RndmEngine.h"

#include <Pythia8/Pythia.h>
#include "Pythia8Plugins/HepMC2.h"

fastsim::Decayer::~Decayer(){;}

fastsim::Decayer::Decayer()
    : pythia_(new Pythia8::Pythia())
    , pythiaRandomEngine_(new gen::P8RndmEngine())
{
    pythia_->setRndmEnginePtr(pythiaRandomEngine_.get());
    pythia_->settings.flag("ProcessLevel:all",false);
    pythia_->settings.flag("PartonLevel:FSRinResonances",false);
    pythia_->settings.flag("ProcessLevel:resonanceDecays",false);
    pythia_->init();

    // forbid all decays
    // (decays are allowed selectively in the decay function)
    Pythia8::ParticleData & pdt = pythia_->particleData;
    int pid = 0;
    while(pdt.nextId(pid) > pid)
    {
    	pid = pdt.nextId(pid);
    	pdt.mayDecay(pid,false);
    }
}

void
fastsim::Decayer::decay(const Particle & particle,std::vector<std::unique_ptr<fastsim::Particle> > & secondaries,CLHEP::HepRandomEngine & engine) const
{
    // make sure pythia takes random numbers from the engine past through via the function arguments
    edm::RandomEngineSentry<gen::P8RndmEngine> sentry(pythiaRandomEngine_.get(), &engine);
    
    // inspired by method Pythia8Hadronizer::residualDecay() in GeneratorInterface/Pythia8Interface/src/Py8GunBase.cc
    int pid = particle.pdgId();
    pythia_->event.reset();

    //std::cout<<"Mother: "<<particle<<std::endl;
    
    Pythia8::Particle pythiaParticle( pid , 93, 0, 0, 0, 0, 0, 0,
				      particle.momentum().X(),
				      particle.momentum().Y(),
				      particle.momentum().Z(),
				      particle.momentum().E(),
				      particle.momentum().M() );
    pythiaParticle.vProd( particle.position().X(), particle.position().Y(), 
			  particle.position().Z(), particle.position().T() );
    pythia_->event.append( pythiaParticle );

    int nentries_before = pythia_->event.size();
    pythia_->particleData.mayDecay(pid,true);   // switch on the decay of this and only this particle (avoid double decays)
    pythia_->next();                            // do the decay
    pythia_->particleData.mayDecay(pid,false);  // switch it off again
    int nentries_after = pythia_->event.size();
    if ( nentries_after <= nentries_before ) return;

    for ( int ipart=nentries_before; ipart<nentries_after; ipart++ ) 
    {
    	Pythia8::Particle& daughter = pythia_->event[ipart];
    	// TODO: check units!!
    	secondaries.emplace_back(new fastsim::Particle(daughter.id()
    						       ,math::XYZTLorentzVector(daughter.xProd(),daughter.yProd(),daughter.zProd(),daughter.tProd())
    						       ,math::XYZTLorentzVector(daughter.px(), daughter.py(), daughter.pz(), daughter.e())));

        //std::cout<<"Decay product: "<<*(secondaries.back())<<std::endl;
    }
    
  return;
}
