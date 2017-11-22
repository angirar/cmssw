import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Modifier_trackingPhase1_cff import trackingPhase1

# import the full tracking equivalent of this file
import RecoTracker.IterativeTracking.DetachedTripletStep_cff as _standard
from FastSimulation.Tracking.SeedingMigration import _hitSetProducerToFactoryPSet

# fast tracking mask producer
import FastSimulation.Tracking.FastTrackerRecHitMaskProducer_cfi
detachedTripletStepMasks = FastSimulation.Tracking.FastTrackerRecHitMaskProducer_cfi.maskProducerFromClusterRemover(_standard.detachedTripletStepClusters)

# tracking regions
detachedTripletStepTrackingRegions = _standard.detachedTripletStepTrackingRegions.clone()

# trajectory seeds
import FastSimulation.Tracking.TrajectorySeedProducer_cfi
detachedTripletStepSeeds = FastSimulation.Tracking.TrajectorySeedProducer_cfi.trajectorySeedProducer.clone(
    layerList = _standard.detachedTripletStepSeedLayers.layerList.value(),
    trackingRegions = "detachedTripletStepTrackingRegions",
    hitMasks = cms.InputTag("detachedTripletStepMasks")
)
 
detachedTripletStepSeeds.seedFinderSelector.CAHitTripletGeneratorFactory = _hitSetProducerToFactoryPSet(_standard.detachedTripletStepHitTriplets)
detachedTripletStepSeeds.seedFinderSelector.CAHitTripletGeneratorFactory.SeedingLayers = cms.InputTag("seedingLayersEDProducer")
detachedTripletStepSeeds.seedFinderSelector.CAHitTripletGeneratorFactory.SeedComparitorPSet.ComponentName = "none"
#detachedTripletStepSeeds.seedFinderSelector.pixelTripletGeneratorFactory = _hitSetProducerToFactoryPSet(_standard.detachedTripletStepHitTriplets)

# track candidates
import FastSimulation.Tracking.TrackCandidateProducer_cfi
detachedTripletStepTrackCandidates = FastSimulation.Tracking.TrackCandidateProducer_cfi.trackCandidateProducer.clone(
    src = cms.InputTag("detachedTripletStepSeeds"),
    MinNumberOfCrossedLayers = 3,
    hitMasks = cms.InputTag("detachedTripletStepMasks")
    )

# tracks 
detachedTripletStepTracks = _standard.detachedTripletStepTracks.clone(TTRHBuilder = 'WithoutRefit')

detachedTripletStepClassifier1 = _standard.detachedTripletStepClassifier1.clone()
detachedTripletStepClassifier1.vertices = "firstStepPrimaryVerticesBeforeMixing"
detachedTripletStepClassifier2 = _standard.detachedTripletStepClassifier2.clone()
detachedTripletStepClassifier2.vertices = "firstStepPrimaryVerticesBeforeMixing"

detachedTripletStep = _standard.detachedTripletStep.clone()
trackingPhase1.toReplaceWith(detachedTripletStep, detachedTripletStepClassifier1.clone(
        GBRForestLabel = 'MVASelectorDetachedTripletStep_Phase1',
        qualityCuts = [-0.2,0.3,0.8],
        ))

# Final sequence 
DetachedTripletStep = cms.Sequence(detachedTripletStepMasks
                                   +detachedTripletStepTrackingRegions
                                   +detachedTripletStepSeeds
                                   +detachedTripletStepTrackCandidates
                                   +detachedTripletStepTracks
                                   +detachedTripletStepClassifier1*detachedTripletStepClassifier2
                                   +detachedTripletStep
                                   )
