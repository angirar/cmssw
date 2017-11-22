import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Modifier_trackingPhase1_cff import trackingPhase1

# import the full tracking equivalent of this file
import RecoTracker.IterativeTracking.InitialStep_cff as _standard
from FastSimulation.Tracking.SeedingMigration import _hitSetProducerToFactoryPSet

# tracking regions
initialStepTrackingRegions = _standard.initialStepTrackingRegions.clone()

# trajectory seeds
import FastSimulation.Tracking.TrajectorySeedProducer_cfi
initialStepSeeds = FastSimulation.Tracking.TrajectorySeedProducer_cfi.trajectorySeedProducer.clone(
    layerList = _standard.initialStepSeedLayers.layerList.value(),
    trackingRegions = "initialStepTrackingRegions"
)

initialStepSeeds.seedFinderSelector.CAHitQuadrupletGeneratorFactory = _hitSetProducerToFactoryPSet(_standard.initialStepHitQuadruplets)
initialStepSeeds.seedFinderSelector.CAHitQuadrupletGeneratorFactory.SeedComparitorPSet.ComponentName = "none"
initialStepSeeds.seedFinderSelector.CAHitQuadrupletGeneratorFactory.SeedingLayers = cms.InputTag("seedingLayersEDProducer")
#initialStepSeeds.seedFinderSelector.pixelTripletGeneratorFactory = _hitSetProducerToFactoryPSet(_standard.initialStepHitTriplets)                                     
#initialStepSeeds.seedFinderSelector.pixelTripletGeneratorFactory.SeedComparitorPSet.ComponentName = "none"

# track candidates
import FastSimulation.Tracking.TrackCandidateProducer_cfi
initialStepTrackCandidates = FastSimulation.Tracking.TrackCandidateProducer_cfi.trackCandidateProducer.clone(
    src = cms.InputTag("initialStepSeeds"),
    MinNumberOfCrossedLayers = 3
    )

# tracks
initialStepTracks = _standard.initialStepTracks.clone(TTRHBuilder = 'WithoutRefit')

firstStepPrimaryVerticesBeforeMixing =  _standard.firstStepPrimaryVertices.clone()

# final selection
initialStepClassifier1 = _standard.initialStepClassifier1.clone()
initialStepClassifier1.vertices = "firstStepPrimaryVerticesBeforeMixing"
#initialStepClassifier2 = _standard.initialStepClassifier2.clone()
#initialStepClassifier2.vertices = "firstStepPrimaryVerticesBeforeMixing"
#initialStepClassifier3 = _standard.initialStepClassifier3.clone()
#initialStepClassifier3.vertices = "firstStepPrimaryVerticesBeforeMixing"


initialStep = _standard.initialStep.clone()
trackingPhase1.toReplaceWith(initialStep, initialStepClassifier1.clone(
        GBRForestLabel = 'MVASelectorInitialStep_Phase1',
        qualityCuts = [-0.95,-0.85,-0.75],
        ))

#Final sequence
InitialStep = cms.Sequence(initialStepTrackingRegions
                           +initialStepSeeds
                           +initialStepTrackCandidates
                           +initialStepTracks                                    
                           +firstStepPrimaryVerticesBeforeMixing
                           +initialStepClassifier1
                           #*initialStepClassifier2*initialStepClassifier3
                           +initialStep
                           )

