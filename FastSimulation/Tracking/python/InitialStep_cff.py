import FWCore.ParameterSet.Config as cms
#from Configuration.Eras.Modifier_trackingPhase1_cff import trackingPhase1
#from RecoPixelVertexing.PixelTriplets.caHitQuadrupletEDProducer_cfi import caHitQuadrupletEDProducer as _caHitQuadrupletEDProducer
# import the full tracking equivalent of this file
import RecoTracker.IterativeTracking.InitialStep_cff as _standard
from FastSimulation.Tracking.SeedingMigration import _hitSetProducerToFactoryPSet

# tracking regions
initialStepTrackingRegions = _standard.initialStepTrackingRegions.clone()
#trackingPhase1.toModify(initialStepTrackingRegions, RegionPSet = dict(ptMin = 0.5))
print "ptMin for trackingPhase1:",initialStepTrackingRegions.RegionPSet.ptMin
# trajectory seeds
import FastSimulation.Tracking.TrajectorySeedProducer_cfi
initialStepSeeds = FastSimulation.Tracking.TrajectorySeedProducer_cfi.trajectorySeedProducer.clone(
    layerList = _standard.initialStepSeedLayers.layerList.value(),
    trackingRegions = "initialStepTrackingRegions"
)
for index in range(len(initialStepSeeds.layerList)):
    print "Layerlist elements:", initialStepSeeds.layerList[index]
 
initialStepSeeds.seedFinderSelector.pixelTripletGeneratorFactory = _hitSetProducerToFactoryPSet(_standard.initialStepHitTriplets)
initialStepSeeds.seedFinderSelector.pixelTripletGeneratorFactory.SeedComparitorPSet.ComponentName = "none"
#initialStepSeeds.seedFinderSelector.MultiHitGeneratorFactory = _hitSetProducerToFactoryPSet(_standard.initialStepHitQuadruplets) 
#initialStepSeeds.seedFinderSelector.MultiHitGeneratorFactory.SeedComparitorPSet.ComponentName = "none"
#initialStepHitTriplets = _standard.initialStepHitTriplets
#initialStepHitQuadruplets = _standard.initialStepHitQuadruplets
#trackingPhase1.toReplaceWith(initialStepHitQuadruplets, _caHitQuadrupletEDProducer.clone(
 #       doublets = "initialStepHitDoublets",
  #      extraHitRPhitolerance = initialStepHitTriplets.extraHitRPhitolerance,
   #     SeedComparitorPSet = initialStepHitTriplets.SeedComparitorPSet,
    #    maxChi2 = dict(
     #       pt1    = 0.7, pt2    = 2,
     #       value1 = 200, value2 = 50,
      #      ),
       # useBendingCorrection = True,
        #fitFastCircle = True,
       # fitFastCircleChi2Cut = True,
        #CAThetaCut = 0.0012,
       # CAPhiCut = 0.2,
        #))

# track candidates
import FastSimulation.Tracking.TrackCandidateProducer_cfi
initialStepTrackCandidates = FastSimulation.Tracking.TrackCandidateProducer_cfi.trackCandidateProducer.clone(
    src = cms.InputTag("initialStepSeeds"),
    MinNumberOfCrossedLayers = 3
    )

# tracks
initialStepTracks = _standard.initialStepTracks.clone(TTRHBuilder = 'WithoutRefit')

firstStepPrimaryVerticesBeforeMixing =  _standard.firstStepPrimaryVertices.clone()
print "firstStepprimaryverticescollection:",firstStepPrimaryVerticesBeforeMixing.TrackLabel

# final selection
initialStepClassifier1 = _standard.initialStepClassifier1.clone()
initialStepClassifier1.vertices = "firstStepPrimaryVerticesBeforeMixing"
print "initialStepClassifier1 vertices:", initialStepClassifier1.vertices
initialStepClassifier2 = _standard.initialStepClassifier2.clone()
initialStepClassifier2.vertices = "firstStepPrimaryVerticesBeforeMixing"
initialStepClassifier3 = _standard.initialStepClassifier3.clone()
initialStepClassifier3.vertices = "firstStepPrimaryVerticesBeforeMixing"


initialStep = _standard.initialStep.clone()
#trackingPhase1.toReplaceWith(initialStep, initialStepClassifier1.clone(
 #       GBRForestLabel = 'MVASelectorInitialStep_Phase1',
  #      qualityCuts = [-0.95,-0.85,-0.75],
   #     ))

# Final sequence
InitialStep = cms.Sequence(initialStepTrackingRegions
                           +initialStepSeeds
                           +initialStepTrackCandidates
                           +initialStepTracks                                    
                           +firstStepPrimaryVerticesBeforeMixing
                           +initialStepClassifier1*initialStepClassifier2*initialStepClassifier3
                           +initialStep
                           )

