#include "FastSimulation/Tracking/interface/SeedFinderSelector.h"

// framework
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
//#include "DataFormats/Common/interface/OwnVector.h"
// track reco
#include "RecoTracker/MeasurementDet/interface/MeasurementTracker.h"
#include "RecoTracker/TkHitPairs/interface/RecHitsSortedInPhi.h"
#include "RecoTracker/TkSeedGenerator/interface/MultiHitGeneratorFromPairAndLayers.h"
#include "RecoTracker/TkSeedGenerator/interface/MultiHitGeneratorFromPairAndLayersFactory.h"
#include "RecoTracker/Record/interface/CkfComponentsRecord.h"
#include "RecoPixelVertexing/PixelTriplets/interface/HitTripletGeneratorFromPairAndLayers.h"
#include "RecoPixelVertexing/PixelTriplets/interface/HitTripletGeneratorFromPairAndLayersFactory.h"
#include "RecoPixelVertexing/PixelTriplets/interface/CAHitTripletGenerator.h"
#include "RecoPixelVertexing/PixelTriplets/interface/CAHitQuadrupletGenerator.h"
#include "RecoPixelVertexing/PixelTriplets/interface/OrderedHitSeeds.h"
#include "FastSimulation/Tracking/interface/CAQuadGeneratorFactory.h"
#include "FastSimulation/Tracking/interface/CATriGeneratorFactory.h"
#include "RecoTracker/TkSeedingLayers/interface/SeedingLayerSetsBuilder.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
// data formats
#include "DataFormats/TrackerRecHit2D/interface/FastTrackerRecHit.h"

SeedFinderSelector::SeedFinderSelector(const edm::ParameterSet & cfg,edm::ConsumesCollector && consumesCollector)
    : trackingRegion_(nullptr)
    , eventSetup_(nullptr)
    , measurementTracker_(nullptr)
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
	CAHitTriplGenerator_.reset(CATriGeneratorFactory::get()->create(tripletConfig.getParameter<std::string>("ComponentName"),tripletConfig,consumesCollector)); 
    }

    if(cfg.exists("CAHitQuadrupletGeneratorFactory"))
    {
        const edm::ParameterSet & quadrupletConfig = cfg.getParameter<edm::ParameterSet>("CAHitQuadrupletGeneratorFactory");
	CAHitQuadGenerator_.reset(CAQuadGeneratorFactory::get()->create(quadrupletConfig.getParameter<std::string>("ComponentName"),quadrupletConfig,consumesCollector));     
	seedingLayers_ = new SeedingLayerSetsBuilder(quadrupletConfig, consumesCollector);
	layerPairs_ = quadrupletConfig.getParameter<std::vector<unsigned>>("layerPairs");
    }

    // if((pixelTripletGenerator_ && multiHitGenerator_) || (CAHitQuadGenerator_ && pixelTripletGenerator_) || (CAHitTriplGenerator_ && multiHitGenerator_))
    // {
    //   throw cms::Exception("FastSimTracking") << "It is forbidden to specify together 'pixelTripletGeneratorFactory', 'CAHitTripletGeneratorFactory' and 'MultiHitGeneratorFactory' in configuration of SeedFinderSelection";
    // }
    // if((pixelTripletGenerator_ && CAHitQuadGenerator_) || (CAHitTriplGenerator_ && CAHitQuadGenerator_) || (CAHitQuadGenerator_ && multiHitGenerator_))
    // {
    //   throw cms::Exception("FastSimTracking") << "It is forbidden to specify 'CAHitQuadrupletGeneratorFactory' together with 'pixelTripletGeneratorFactory', 'CAHitTripletGeneratorFactory' or 'MultiHitGeneratorFactory' in configuration of SeedFinderSelection";
    // }

}


SeedFinderSelector::~SeedFinderSelector(){;}

void SeedFinderSelector::initEvent(const edm::Event & ev,const edm::EventSetup & es)
{
    eventSetup_ = &es;
    
    edm::ESHandle<MeasurementTracker> measurementTrackerHandle;
    es.get<CkfComponentsRecord>().get(measurementTrackerLabel_, measurementTrackerHandle);
    es.get<TrackerTopologyRcd>().get(trackerTopology);
    measurementTracker_ = &(*measurementTrackerHandle);

    if(multiHitGenerator_)
    {
        multiHitGenerator_->initES(es);
    }
    if(CAHitQuadGenerator_){
      std::cout<<"CAHit generator enabled"<<std::endl;
      seedingLayer = seedingLayers_->hits(ev, es);
      CAHitQuadGenerator_->initEvent(ev,es);
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
    
    if(result.empty())
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
	  std::cout<<"Inside PixelTripletGenerator"<<std::endl;
	    OrderedHitTriplets tripletresult;
	    pixelTripletGenerator_->hitTriplets(*trackingRegion_,tripletresult,*eventSetup_,result,&thmp,thirdLayerDetLayer,1);
	    return !tripletresult.empty();
	}
	else if(multiHitGenerator_)
	{
	    OrderedMultiHits  tripletresult;
	    multiHitGenerator_->hitTriplets(*trackingRegion_,tripletresult,*eventSetup_,result,&thmp,thirdLayerDetLayer,1);
	    return !tripletresult.empty();
	}
	else if(CAHitTriplGenerator_)
	{  
	  return true;
	}
    }

    if(CAHitQuadGenerator_)
    {
      std::cout<<"Inside CAHit section"<<std::endl;
      if(hits.size() < 4)
	{
	  throw cms::Exception("FastSimTracking") << "For the given configuration, SeedFinderSelector::pass requires at least 4 hits";
        }

      if(!seedingLayer)
	std::cout<<"SeedingLayers pointer not set"<<std::endl;

      SeedingLayerSetsHits & layers = *seedingLayer;

      for(int i=0; i<(int)hits.size()-1; i++){
        //-----------------determining hit layer---------------                                                                                                                
	std::string hitlayer[2] = {};
        int layerNo = -1;
	std::string side;
        bool IsPixB = false;
	SeedingLayerSetsHits::SeedingLayerSet pairCandidate;
        //hit 1                                                                                                                                                                
        if( (hits[i]->det()->geographicalId()).subdetId() == PixelSubdetector::PixelBarrel){
          layerNo = (*trackerTopology.product()).pxbLayer(hits[i]->det()->geographicalId());
          IsPixB = true;
        }
        else if ((hits[i]->det()->geographicalId()).subdetId() == PixelSubdetector::PixelEndcap){
          layerNo = (*trackerTopology.product()).pxfDisk(hits[i]->det()->geographicalId());
          side = (*trackerTopology.product()).pxfSide(hits[i]->det()->geographicalId())==1 ? "_neg" : "_pos";
          IsPixB = false;
        }
        hitlayer[0] = LayerName(layerNo, side, IsPixB);
        //hit 2                                                                                                                                                                
        if( (hits[i+1]->det()->geographicalId()).subdetId() == PixelSubdetector::PixelBarrel){
          layerNo = (*trackerTopology.product()).pxbLayer(hits[i+1]->det()->geographicalId());
          IsPixB = true;
        }
        else if ((hits[i+1]->det()->geographicalId()).subdetId() == PixelSubdetector::PixelEndcap){
          layerNo = (*trackerTopology.product()).pxfDisk(hits[i+1]->det()->geographicalId());
          side = (*trackerTopology.product()).pxfSide(hits[i+1]->det()->geographicalId())==1 ? "_neg" : "_pos";
          IsPixB = false;
        }
	hitlayer[1] = LayerName(layerNo, side, IsPixB);
        for(SeedingLayerSetsHits::SeedingLayerSet ls : *seedingLayer){
          for(const auto p : layerPairs_){
            pairCandidate = ls.slice(p,p+2);
	    std::string layerPair[2] = {};
            int i=0;
            for(auto layerSet : pairCandidate){
              layerPair[i] = layerSet.name();
              i++;
            }
            if((layerPair[0] == hitlayer[0] && layerPair[1] == hitlayer[1]))
              break;
          }
        }
      }

      return true;      
    }
    return true;
}
std::string SeedFinderSelector::LayerName(int layerN, std::string layerside, bool IsPixBarrel) const
{
  std::string layerName = "UNKNWN";
  if(IsPixBarrel){
    if(layerN == 1)
      layerName = "BPix1";
    if(layerN == 2)
      layerName = "BPix2";
    if(layerN == 3)
      layerName = "BPix3";
    if(layerN == 4)
      layerName = "BPix4";
  }
  else{
    if(layerN == 1)
      layerName = "FPix1"+layerside;
    if(layerN == 2)
      layerName = "FPix2"+layerside;
    if(layerN == 3)
      layerName = "FPix3"+layerside;
  }
  return layerName;
}
