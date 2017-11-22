#include "FastSimulation/Tracking/interface/SeedFinderSelector.h"

// framework
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// track reco
#include "RecoTracker/MeasurementDet/interface/MeasurementTracker.h"
#include "RecoTracker/TkHitPairs/interface/RecHitsSortedInPhi.h"
#include "RecoTracker/TkHitPairs/interface/HitPairGeneratorFromLayerPair.h"
#include "RecoTracker/TkSeedGenerator/interface/MultiHitGeneratorFromPairAndLayers.h"
#include "RecoTracker/TkSeedGenerator/interface/MultiHitGeneratorFromPairAndLayersFactory.h"
#include "RecoTracker/Record/interface/CkfComponentsRecord.h"
#include "RecoPixelVertexing/PixelTriplets/interface/HitTripletGeneratorFromPairAndLayers.h"
#include "RecoPixelVertexing/PixelTriplets/interface/HitTripletGeneratorFromPairAndLayersFactory.h"
#include "RecoPixelVertexing/PixelTriplets/plugins/CAHitTripletGenerator.h"
#include "RecoPixelVertexing/PixelTriplets/plugins/CAHitQuadrupletGenerator.h"
#include "RecoPixelVertexing/PixelTriplets/interface/OrderedHitSeeds.h"
#include "RecoPixelVertexing/PixelTriplets/interface/HitQuadrupletGenerator.h"
#include "RecoPixelVertexing/PixelTriplets/interface/HitTripletGenerator.h"
#include "RecoPixelVertexing/PixelTriplets/interface/HitQuadrupletGeneratorFactory.h"
#include "RecoPixelVertexing/PixelTriplets/interface/HitTripletGeneratorFactory.h"

// data formats
#include "DataFormats/TrackerRecHit2D/interface/FastTrackerRecHit.h"

SeedFinderSelector::SeedFinderSelector(const edm::ParameterSet & cfg,edm::ConsumesCollector && consumesCollector)
    : trackingRegion_(0)
    , ev_(0)
    , eventSetup_(0)
    , measurementTracker_(0)
    , measurementTrackerLabel_(cfg.getParameter<std::string>("measurementTracker"))
{  
    if(cfg.exists("pixelTripletGeneratorFactory"))
    {
        const edm::ParameterSet & tripletConfig = cfg.getParameter<edm::ParameterSet>("pixelTripletGeneratorFactory");
        pixelTripletGenerator_.reset(HitTripletGeneratorFromPairAndLayersFactory::get()->create(tripletConfig.getParameter<std::string>("ComponentName"),tripletConfig,consumesCollector));
    }

    if(cfg.exists("MultiHitGeneratorFactory"))
    {
        const edm::ParameterSet & tripletConfig = cfg.getParameter<edm::ParameterSet>("MultiHitGeneratorFactory");
        multiHitGenerator_.reset(MultiHitGeneratorFromPairAndLayersFactory::get()->create(tripletConfig.getParameter<std::string>("ComponentName"),tripletConfig));
    }

    if(cfg.exists("CAHitTripletGeneratorFactory"))
    {
        const edm::ParameterSet & tripletConfig = cfg.getParameter<edm::ParameterSet>("CAHitTripletGeneratorFactory");
	CAHitTriplGenerator_.reset(HitTripletGeneratorFactory::get()->create(tripletConfig.getParameter<std::string>("ComponentName"),tripletConfig,consumesCollector));
    }
    if(cfg.exists("CAHitQuadrupletGeneratorFactory"))
    {
        const edm::ParameterSet & quadrupletConfig = cfg.getParameter<edm::ParameterSet>("CAHitQuadrupletGeneratorFactory");
        CAHitQuadGenerator_.reset(HitQuadrupletGeneratorFactory::get()->create(quadrupletConfig.getParameter<std::string>("ComponentName"),quadrupletConfig,consumesCollector));
    }

    if((pixelTripletGenerator_ && multiHitGenerator_) || (CAHitTriplGenerator_ && pixelTripletGenerator_) || (CAHitTriplGenerator_ && multiHitGenerator_))
    {
	throw cms::Exception("FastSimTracking") << "It is forbidden to specify together 'pixelTripletGeneratorFactory', 'CAHitTripletGeneratorFactory' and 'MultiHitGeneratorFactory' in configuration of SeedFinderSelection";
    }
    if((pixelTripletGenerator_ && CAHitQuadGenerator_) || (CAHitTriplGenerator_ && CAHitQuadGenerator_) || (CAHitQuadGenerator_ && multiHitGenerator_))
    {
      throw cms::Exception("FastSimTracking") << "It is forbidden to specify 'CAHitQuadrupletGeneratorFactory' together with 'pixelTripletGeneratorFactory', 'CAHitTripletGeneratorFactory' or 'MultiHitGeneratorFactory' in configuration of SeedFinderSelection";
    }
}


SeedFinderSelector::~SeedFinderSelector(){;}

void SeedFinderSelector::initEvent(const edm::Event & ev,const edm::EventSetup & es)
{
    eventSetup_ = &es;
    
    edm::ESHandle<MeasurementTracker> measurementTrackerHandle;
    es.get<CkfComponentsRecord>().get(measurementTrackerLabel_, measurementTrackerHandle);
    measurementTracker_ = &(*measurementTrackerHandle);

    if(multiHitGenerator_)
    {
        multiHitGenerator_->initES(es);
    }
}


bool SeedFinderSelector::pass(const std::vector<const FastTrackerRecHit *>& hits) const
{
    if(!measurementTracker_ || !eventSetup_)
    {
	throw cms::Exception("FastSimTracking") << "ERROR: event not initialized";
    }
    if(!trackingRegion_)
    {
	throw cms::Exception("FastSimTracking") << "ERROR: trackingRegion not set";
    }


    // check the inner 2 hits
    if(hits.size() < 2)
    {
	throw cms::Exception("FastSimTracking") << "SeedFinderSelector::pass requires at least 2 hits";
    }
    const DetLayer * firstLayer = measurementTracker_->geometricSearchTracker()->detLayer(hits[0]->det()->geographicalId());
    const DetLayer * secondLayer = measurementTracker_->geometricSearchTracker()->detLayer(hits[1]->det()->geographicalId());
    
    std::vector<BaseTrackerRecHit const *> firstHits(1,static_cast<const BaseTrackerRecHit*>(hits[0]));
    std::vector<BaseTrackerRecHit const *> secondHits(1,static_cast<const BaseTrackerRecHit*>(hits[1]));
    
    const RecHitsSortedInPhi fhm(firstHits, trackingRegion_->origin(), firstLayer);
    const RecHitsSortedInPhi shm(secondHits, trackingRegion_->origin(), secondLayer);
    
    HitDoublets result(fhm,shm);
    HitPairGeneratorFromLayerPair::doublets(*trackingRegion_,*firstLayer,*secondLayer,fhm,shm,*eventSetup_,0,result);
    
    if(result.size()==0)
    {
	return false;
    }
    
    // check the inner 3 hits
    if(pixelTripletGenerator_ || multiHitGenerator_ || CAHitTriplGenerator_)
    {
      if(hits.size() < 3)
	{
	    throw cms::Exception("FastSimTracking") << "For the given configuration, SeedFinderSelector::pass requires at least 3 hits";
	}
	const DetLayer * thirdLayer = measurementTracker_->geometricSearchTracker()->detLayer(hits[2]->det()->geographicalId());
	std::vector<const DetLayer *> thirdLayerDetLayer(1,thirdLayer);
	std::vector<BaseTrackerRecHit const *> thirdHits(1,static_cast<const BaseTrackerRecHit*>(hits[2]));
	const RecHitsSortedInPhi thm(thirdHits,trackingRegion_->origin(), thirdLayer);
	const RecHitsSortedInPhi * thmp =&thm;
	
	if(pixelTripletGenerator_)
	{
	  OrderedHitTriplets tripletresult;
	  pixelTripletGenerator_->hitTriplets(*trackingRegion_,tripletresult,*eventSetup_,result,&thmp,thirdLayerDetLayer,1);
	  return tripletresult.size()!=0;
	}
	else if(multiHitGenerator_)
	{
	  OrderedMultiHits  tripletresult;
	  multiHitGenerator_->hitTriplets(*trackingRegion_,tripletresult,*eventSetup_,result,&thmp,thirdLayerDetLayer,1);
	  return tripletresult.size()!=0;
	}
	else if(CAHitTriplGenerator_)
	{
	  // OrderedHitTriplets tripletresult;
	  // CAHitTriplGenerator_->hitTriplets(*trackingRegion_,tripletresult,*ev_,*eventSetup_);
	  // return tripletresult.size()!=0;
	  return true;
	}
    }
    if(CAHitQuadGenerator_)
    {
      if(hits.size() < 4)
	{
            throw cms::Exception("FastSimTracking") << "For the given configuration, SeedFinderSelector::pass requires at least 4 hits";
	}
       
      // OrderedHitSeeds quadrupletresult;
      // CAHitQuadGenerator_->hitQuadruplets(*trackingRegion_,quadrupletresult,*ev_,*eventSetup_);
      // return quadrupletresult.size()!=0;
      return true;
    }
    
    return true;
    
}
