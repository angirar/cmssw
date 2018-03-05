// Plugin definition for the algorithm

#include "FastSimulation/Tracking/interface/CAQuadGeneratorFactory.h"
#include "FastSimulation/Tracking/interface/CATriGeneratorFactory.h"
#include "RecoPixelVertexing/PixelTriplets/interface/CAHitQuadrupletGenerator.h"
#include "RecoPixelVertexing/PixelTriplets/interface/CAHitTripletGenerator.h"

DEFINE_EDM_PLUGIN( CAQuadGeneratorFactory, CAHitQuadrupletGenerator, "CAHitQuadrupletGenerator" );
DEFINE_EDM_PLUGIN( CATriGeneratorFactory, CAHitTripletGenerator, "CAHitTripletGenerator" );
