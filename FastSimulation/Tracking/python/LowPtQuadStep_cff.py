import FWCore.ParameterSet.Config as cms

# import the full tracking equivalent of this file                                                                                                                          
import RecoTracker.IterativeTracking.LowPtQuadStep_cff as _standard
from FastSimulation.Tracking.SeedingMigration import _hitSetProducerToFactoryPSet

# fast tracking mask producer                                                                                                                                                  
import FastSimulation.Tracking.FastTrackerRecHitMaskProducer_cfi
lowPtQuadStepMasks = FastSimulation.Tracking.FastTrackerRecHitMaskProducer_cfi.maskProducerFromClusterRemover(_standard.lowPtQuadStepClusters)

# tracking regions                                                                                                                                                          
lowPtQuadStepTrackingRegions = _standard.lowPtQuadStepTrackingRegions.clone()

# trajectory seeds                                                                                                                                                             
import FastSimulation.Tracking.TrajectorySeedProducer_cfi
lowPtQuadStepSeeds = FastSimulation.Tracking.TrajectorySeedProducer_cfi.trajectorySeedProducer.clone(
    layerList = _standard.lowPtQuadStepSeedLayers.layerList.value(),
    trackingRegions = "lowPtQuadStepTrackingRegions",
    hitMasks = cms.InputTag("lowPtQuadStepMasks"),
)
                        
lowPtQuadStepSeeds.seedFinderSelector.CAHitQuadrupletGeneratorFactory = _hitSetProducerToFactoryPSet(_standard.lowPtQuadStepHitQuadruplets)
lowPtQuadStepSeeds.seedFinderSelector.CAHitQuadrupletGeneratorFactory.SeedComparitorPSet.ComponentName = "none"
lowPtQuadStepSeeds.seedFinderSelector.CAHitQuadrupletGeneratorFactory.SeedingLayers = cms.InputTag("seedingLayersEDProducer")
# track candidates                                                                                                                                                             
import FastSimulation.Tracking.TrackCandidateProducer_cfi
lowPtQuadStepTrackCandidates = FastSimulation.Tracking.TrackCandidateProducer_cfi.trackCandidateProducer.clone(
    src = cms.InputTag("lowPtQuadStepSeeds"),
    MinNumberOfCrossedLayers = 3,
    hitMasks = cms.InputTag("lowPtQuadStepMasks"),
)

# tracks                                                                                                                                                                    
lowPtQuadStepTracks = _standard.lowPtQuadStepTracks.clone(TTRHBuilder = 'WithoutRefit')

# final selection  
lowPtQuadStep = _standard.lowPtQuadStep.clone()
lowPtQuadStep.vertices = "firstStepPrimaryVerticesBeforeMixing"

# Final swquence                                                                                                                                                               
LowPtQuadStep = cms.Sequence(lowPtQuadStepMasks
                                +lowPtQuadStepTrackingRegions
                                +lowPtQuadStepSeeds
                                +lowPtQuadStepTrackCandidates
                                +lowPtQuadStepTracks
                                +lowPtQuadStep
                                )
