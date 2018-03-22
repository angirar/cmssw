#ifndef SEEDFINDERSELECTOR_H
#define SEEDFINDERSELECTOR_H

#include <vector>
#include <memory>
#include <string>
#include "FWCore/Utilities/interface/RunningAverage.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "RecoTracker/TkHitPairs/interface/HitPairGeneratorFromLayerPair.h"
#include "TrackingTools/TransientTrackingRecHit/interface/SeedingLayerSetsHits.h"
#include "RecoTracker/TkHitPairs/interface/IntermediateHitDoublets.h"

class TrackingRegion;
class FastTrackerRecHit;
class MultiHitGeneratorFromPairAndLayers;
class HitTripletGeneratorFromPairAndLayers;
class MeasurementTracker;
class CAHitTripletGenerator;
class CAHitQuadrupletGenerator;
class SeedingLayerSetsBuilder;

namespace edm
{
    class Event;
    class EventSetup;
    class ParameterSet;
    class ConsumesCollector;
}
namespace IHD
{
  class ImplBase;
}

class SeedFinderSelector
{
public:

    SeedFinderSelector(const edm::ParameterSet & cfg,edm::ConsumesCollector && consumesCollector);
    
    ~SeedFinderSelector();

    void initEvent(const edm::Event & ev,const edm::EventSetup & es);

    void setTrackingRegion(const TrackingRegion * trackingRegion){trackingRegion_ = trackingRegion;}
    
    bool pass(const std::vector<const FastTrackerRecHit *>& hits) const;
    std::string LayerName(int layerN, std::string layerside, bool IsPixBarrel) const;    

private:
    
    std::unique_ptr<HitTripletGeneratorFromPairAndLayers> pixelTripletGenerator_;
    std::unique_ptr<MultiHitGeneratorFromPairAndLayers> multiHitGenerator_;
    std::unique_ptr<CAHitTripletGenerator> CAHitTriplGenerator_;
    std::unique_ptr<CAHitQuadrupletGenerator> CAHitQuadGenerator_;
    const TrackingRegion * trackingRegion_;
    const edm::EventSetup * eventSetup_;
    const MeasurementTracker * measurementTracker_;
    const std::string measurementTrackerLabel_;
    SeedingLayerSetsBuilder* seedingLayers_;
    std::unique_ptr<SeedingLayerSetsHits> seedingLayer;
    std::vector<unsigned> layerPairs_;
    edm::ESHandle<TrackerTopology> trackerTopology;
};

namespace IHD {
  class ImplBase {
  public:
    ImplBase(const edm::ParameterSet& iConfig);
    virtual ~ImplBase() = default;


    virtual void produce(const SeedingLayerSetsHits& layers, SeedingLayerSetsHits::SeedingLayerSet layerpair, const TrackingRegion& region, HitDoublets && doublets, edm::Event & iEvent) = 0;

  protected:
    edm::RunningAverage localRA_;
  };
  ImplBase::ImplBase(const edm::ParameterSet& iConfig)
    {
    }

  template <typename T_SeedingHitSets, typename T_IntermediateHitDoublets>
    struct Impl: public ImplBase {
  Impl(const edm::ParameterSet& iConfig): ImplBase(iConfig) {}
    ~Impl() override = default;

    void produce(const SeedingLayerSetsHits& layers, SeedingLayerSetsHits::SeedingLayerSet layerpair, const TrackingRegion& region, HitDoublets && doublets, edm::Event& iEvent) override {
      auto seedingHitSetsProducer = T_SeedingHitSets(&localRA_);
      auto intermediateHitDoubletsProducer = T_IntermediateHitDoublets(&layers);
      /* seedingHitSetsProducer.reserve(1); */
      /* intermediateHitDoubletsProducer.reserve(1); */
      auto hitCachePtr_filler_shs = seedingHitSetsProducer.beginRegion(&region, nullptr);
      auto hitCachePtr_filler_ihd = intermediateHitDoubletsProducer.beginRegion(&region, std::get<0>(hitCachePtr_filler_shs));
      //auto hitCachePtr = std::get<0>(hitCachePtr_filler_ihd);                                                                                                                
      if(doublets.empty()) return;
      //seedingHitSetsProducer.fill(std::get<1>(hitCachePtr_filler_shs), doublets);                                                                                            
      intermediateHitDoubletsProducer.fill(std::get<1>(hitCachePtr_filler_ihd), layerpair, std::move(doublets));
      //      seedingHitSetsProducer.put(iEvent);                                                                                                                              
      //      intermediateHitDoubletsProducer.put(iEvent);                                                                                                                     
    }
  };

  class DoNothing {
  public:
    DoNothing(const SeedingLayerSetsHits *) {}
    DoNothing(edm::RunningAverage *) {}

    void reserve(size_t) {}

    auto beginRegion(const TrackingRegion *, LayerHitMapCache *ptr) {
      return std::make_tuple(ptr, 0);
    }

    void fill(int, const HitDoublets&) {}
    void fill(int, const SeedingLayerSetsHits::SeedingLayerSet&, HitDoublets&&) {}
    /* void put(edm::Event&) {} */
    /* void putEmpty(edm::Event&) {} */
  };

  class ImplIntermediateHitDoublets {
  public:
  ImplIntermediateHitDoublets(const SeedingLayerSetsHits *layers):
    intermediateHitDoublets_(std::make_unique<IntermediateHitDoublets>(layers)),
      layers_(layers)
      {}

    void reserve(size_t regionsSize) {
      intermediateHitDoublets_->reserve(regionsSize, layers_->size());
    }

    auto beginRegion(const TrackingRegion *region, LayerHitMapCache *) {
      auto filler = intermediateHitDoublets_->beginRegion(region);
      return std::make_tuple(&(filler.layerHitMapCache()), std::move(filler));
    }
    void fill(IntermediateHitDoublets::RegionFiller& filler, const SeedingLayerSetsHits::SeedingLayerSet& layerSet, HitDoublets&& doublets) {
      filler.addDoublets(layerSet, std::move(doublets));
    }

    /* void put(edm::Event& iEvent) { */
    /*   intermediateHitDoublets_->shrink_to_fit(); */
    /*   putEmpty(iEvent); */
    /* } */

    /* void putEmpty(edm::Event& iEvent) { */
    /*   iEvent.put(std::move(intermediateHitDoublets_)); */
    /* } */

  private:
    std::unique_ptr<IntermediateHitDoublets> intermediateHitDoublets_;
    const SeedingLayerSetsHits *layers_;
  };
}



#endif
