#include "RecoLocalCalo/HGCalRecProducers/plugins/HGCalUncalibRecHitWorkerWeights.h"

#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

void configureIt(const edm::ParameterSet& conf, HGCalUncalibRecHitRecWeightsAlgo<HGCalDataFrame>& maker) {
  constexpr char isSiFE[] = "isSiFE";
  constexpr char adcNbits[] = "adcNbits";
  constexpr char adcSaturation[] = "adcSaturation";
  constexpr char tdcNbits[] = "tdcNbits";
  constexpr char tdcSaturation[] = "tdcSaturation";
  constexpr char tdcOnset[] = "tdcOnset";
  constexpr char toaLSB_ns[] = "toaLSB_ns";
  constexpr char fCPerMIP[] = "fCPerMIP";

  if (conf.exists(isSiFE)) {
    maker.set_isSiFESim(conf.getParameter<bool>(isSiFE));
  } else {
    maker.set_isSiFESim(false);
  }

  if (conf.exists(adcNbits)) {
    uint32_t nBits = conf.getParameter<uint32_t>(adcNbits);
    double saturation = conf.getParameter<double>(adcSaturation);
    float adcLSB = saturation / pow(2., nBits);
    maker.set_ADCLSB(adcLSB);
  } else {
    maker.set_ADCLSB(-1.);
  }

  if (conf.exists(tdcNbits)) {
    uint32_t nBits = conf.getParameter<uint32_t>(tdcNbits);
    double saturation = conf.getParameter<double>(tdcSaturation);
    double onset = conf.getParameter<double>(tdcOnset);  // in fC
    float tdcLSB = saturation / pow(2., nBits);
    maker.set_TDCLSB(tdcLSB);
    maker.set_tdcOnsetfC(onset);
  } else {
    maker.set_TDCLSB(-1.);
    maker.set_tdcOnsetfC(-1.);
  }

  if (conf.exists(toaLSB_ns)) {
    maker.set_toaLSBToNS(conf.getParameter<double>(toaLSB_ns));
  } else {
    maker.set_toaLSBToNS(-1.);
  }

  if (conf.exists(fCPerMIP)) {
    maker.set_fCPerMIP(conf.getParameter<std::vector<double> >(fCPerMIP));
  } else {
    maker.set_fCPerMIP(std::vector<double>({1.0}));
  }
}

HGCalUncalibRecHitWorkerWeights::HGCalUncalibRecHitWorkerWeights(const edm::ParameterSet& ps, edm::ConsumesCollector iC)
    : HGCalUncalibRecHitWorkerBaseClass(ps, iC),
  ee_geometry_token_(iC.esConsumes(edm::ESInputTag("", "HGCalEESensitive"))),
  hef_geometry_token_(iC.esConsumes(edm::ESInputTag("", "HGCalHESiliconSensitive"))),
  hfnose_geometry_token_(iC.esConsumes(edm::ESInputTag("", "HGCalHFNoseSensitive"))) {
  const edm::ParameterSet& ee_cfg = ps.getParameterSet("HGCEEConfig");
  const edm::ParameterSet& hef_cfg = ps.getParameterSet("HGCHEFConfig");
  const edm::ParameterSet& heb_cfg = ps.getParameterSet("HGCHEBConfig");
  const edm::ParameterSet& hfnose_cfg = ps.getParameterSet("HGCHFNoseConfig");
  configureIt(ee_cfg, uncalibMaker_ee_);
  configureIt(hef_cfg, uncalibMaker_hef_);
  configureIt(heb_cfg, uncalibMaker_heb_);
  configureIt(hfnose_cfg, uncalibMaker_hfnose_);
}

void HGCalUncalibRecHitWorkerWeights::set(const edm::EventSetup& es) {
  if (uncalibMaker_ee_.isSiFESim()) {
    const HGCalGeometry& hgceeGeo = es.getData(ee_geometry_token_);
    uncalibMaker_ee_.setGeometry(hgceeGeo);
  }
  if (uncalibMaker_hef_.isSiFESim()) {
    edm::ESHandle<HGCalGeometry> hgchefGeoHandle = es.getHandle(hef_geometry_token_);
    uncalibMaker_hef_.setGeometry(hgchefGeoHandle.product());
  }
  uncalibMaker_heb_.setGeometry(nullptr);
  if (uncalibMaker_hfnose_.isSiFESim()) {
    edm::ESHandle<HGCalGeometry> hgchfnoseGeoHandle = es.getHandle(hfnose_geometry_token_);
    uncalibMaker_hfnose_.setGeometry(hgchfnoseGeoHandle.product());
  }
}

bool HGCalUncalibRecHitWorkerWeights::runHGCEE(const HGCalDigiCollection::const_iterator& itdg,
                                               HGCeeUncalibratedRecHitCollection& result) {
  result.push_back(uncalibMaker_ee_.makeRecHit(*itdg));
  return true;
}

bool HGCalUncalibRecHitWorkerWeights::runHGCHEsil(const HGCalDigiCollection::const_iterator& itdg,
                                                  HGChefUncalibratedRecHitCollection& result) {
  result.push_back(uncalibMaker_hef_.makeRecHit(*itdg));
  return true;
}

bool HGCalUncalibRecHitWorkerWeights::runHGCHEscint(const HGCalDigiCollection::const_iterator& itdg,
                                                    HGChebUncalibratedRecHitCollection& result) {
  result.push_back(uncalibMaker_heb_.makeRecHit(*itdg));
  return true;
}

bool HGCalUncalibRecHitWorkerWeights::runHGCHFNose(const HGCalDigiCollection::const_iterator& itdg,
                                                   HGChfnoseUncalibratedRecHitCollection& result) {
  result.push_back(uncalibMaker_hfnose_.makeRecHit(*itdg));
  return true;
}

#include "FWCore/Framework/interface/MakerMacros.h"
#include "RecoLocalCalo/HGCalRecProducers/interface/HGCalUncalibRecHitWorkerFactory.h"
DEFINE_EDM_PLUGIN(HGCalUncalibRecHitWorkerFactory, HGCalUncalibRecHitWorkerWeights, "HGCalUncalibRecHitWorkerWeights");
