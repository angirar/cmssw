#ifndef FastSimulation_Tracking_CAQuadGeneratorFactory_H 
#define FastSimulation_Tracking_CAQuadGeneratorFactory_H

#include "RecoPixelVertexing/PixelTriplets/interface/CAHitQuadrupletGenerator.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"

namespace edm {class ParameterSet; class ConsumesCollector;}

typedef edmplugin::PluginFactory<CAHitQuadrupletGenerator *(const edm::ParameterSet &, edm::ConsumesCollector&)>
		  CAQuadGeneratorFactory;

#endif
