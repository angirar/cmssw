#ifndef FastSimulation_Tracking_CATriGeneratorFactory_H 
#define FastSimulation_Tracking_CATriGeneratorFactory_H

#include "RecoPixelVertexing/PixelTriplets/interface/CAHitTripletGenerator.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"

namespace edm {class ParameterSet; class ConsumesCollector;}

typedef edmplugin::PluginFactory<CAHitTripletGenerator *(const edm::ParameterSet &, edm::ConsumesCollector&)>
		  CATriGeneratorFactory;

#endif
