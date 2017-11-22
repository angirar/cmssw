#ifndef HitQuadrupletGeneratorFactory_H
#define HitQuadrupletGeneratorFactory_H

#include "RecoPixelVertexing/PixelTriplets/interface/HitQuadrupletGenerator.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"

namespace edm {class ParameterSet; class ConsumesCollector;}
 
typedef edmplugin::PluginFactory<HitQuadrupletGenerator *(const edm::ParameterSet &, edm::ConsumesCollector&)>
		  HitQuadrupletGeneratorFactory;
#endif
