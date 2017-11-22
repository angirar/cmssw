#ifndef HitTripletGeneratorFactory_H
#define HitTripletGeneratorFactory_H

#include "RecoPixelVertexing/PixelTriplets/interface/HitTripletGenerator.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"

namespace edm {class ParameterSet; class ConsumesCollector;}
 
typedef edmplugin::PluginFactory<HitTripletGenerator *(const edm::ParameterSet &, edm::ConsumesCollector&)>
		  HitTripletGeneratorFactory;
#endif
