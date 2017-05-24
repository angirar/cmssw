import FWCore.ParameterSet.Config as cms
from RecoPixelVertexing.PixelTriplets.caHitTripletEDProducer_cfi import caHitTripletEDProducer as _caHitTripletEDProducer
from Configuration.Eras.Modifier_trackingPhase1_cff import trackingPhase1
# import the full tracking equivalent of this file                                                                                                                           
import RecoTracker.IterativeTracking.HighPtTripletStep_cff as _standard
from FastSimulation.Tracking.SeedingMigration import _hitSetProducerToFactoryPSet

# fast tracking mask producer                                                                                                                                              
import FastSimulation.Tracking.FastTrackerRecHitMaskProducer_cfi
highPtTripletStepMasks = FastSimulation.Tracking.FastTrackerRecHitMaskProducer_cfi.maskProducerFromClusterRemover(_standard.highPtTripletStepClusters)

# tracking regions                                                                                                                                                            
highPtTripletStepTrackingRegions = _standard.highPtTripletStepTrackingRegions.clone()

# trajectory seeds                                                                                                                                                         
import FastSimulation.Tracking.TrajectorySeedProducer_cfi
highPtTripletStepSeeds = FastSimulation.Tracking.TrajectorySeedProducer_cfi.trajectorySeedProducer.clone(
    layerList = _standard.highPtTripletStepSeedLayers.layerList.value(),
    trackingRegions = "highPtTripletStepTrackingRegions",
    hitMasks = cms.InputTag("highPtTripletStepMasks"),
)

from RecoPixelVertexing.PixelTriplets.pixelTripletHLTEDProducer_cfi import pixelTripletHLTEDProducer as _pixelTripletHLTEDProducer
from RecoPixelVertexing.PixelLowPtUtilities.ClusterShapeHitFilterESProducer_cfi import *
import RecoPixelVertexing.PixelLowPtUtilities.LowPtClusterShapeSeedComparitor_cfi
highPtTripletStepHitTriplets = _pixelTripletHLTEDProducer.clone(
    doublets = "highPtTripletStepHitDoublets",
    produceSeedingHitSets = True,
    SeedComparitorPSet = RecoPixelVertexing.PixelLowPtUtilities.LowPtClusterShapeSeedComparitor_cfi.LowPtClusterShapeSeedComparitor.clone()
)

highPtTripletStepSeeds.seedFinderSelector.pixelTripletGeneratorFactory = _hitSetProducerToFactoryPSet(_standard.highPtTripletStepHitTriplets)
trackingPhase1.toReplaceWith(highPtTripletStepHitTriplets,_caHitTripletEDProducer.clone(
        doublets = "highPtTripletStepHitDoublets",
        extraHitRPhitolerance = highPtTripletStepHitTriplets.extraHitRPhitolerance,
        SeedComparitorPSet = highPtTripletStepHitTriplets.SeedComparitorPSet,
        maxChi2 = dict(
            pt1    = 0.8, pt2    = 8,
            value1 = 100, value2 = 6,
            ),
        useBendingCorrection = True,
        CAThetaCut = 0.004,
        CAPhiCut = 0.07,
        CAHardPtCut = 0.3,
))
#highPtTripletStepSeeds.pixelTripletGeneratorFactory.SeedComparitorPSet=cms.PSet(  ComponentName = cms.string( "none" ) )                                                      

# track candidates                                                                                                                                                             
import FastSimulation.Tracking.TrackCandidateProducer_cfi
highPtTripletStepTrackCandidates = FastSimulation.Tracking.TrackCandidateProducer_cfi.trackCandidateProducer.clone(
    src = cms.InputTag("highPtTripletStepSeeds"),
    MinNumberOfCrossedLayers = 3,
    hitMasks = cms.InputTag("highPtTripletStepMasks"),
)

# tracks                                                                                                                                                                     
highPtTripletStepTracks = _standard.highPtTripletStepTracks.clone(TTRHBuilder = 'WithoutRefit')

# final selection                                                                                                                                                           
highPtTripletStep = _standard.highPtTripletStep.clone()
highPtTripletStep.vertices = "firstStepPrimaryVerticesBeforeMixing"

# Final sequence           
HighPtTripletStep = cms.Sequence(highPtTripletStepMasks
                                +highPtTripletStepTrackingRegions
                                +highPtTripletStepSeeds
                                +highPtTripletStepTrackCandidates
                                +highPtTripletStepTracks
                                +highPtTripletStep
                                )

