#include "DQM/EcalCommon/interface/DQWorker.h"

#include "DQM/EcalCommon/interface/MESetUtils.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/Provenance/interface/EventID.h"

#include "Geometry/EcalMapping/interface/EcalMappingRcd.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"

namespace ecaldqm {
  DQWorker::DQWorker()
      : name_(""),
        MEs_(),
        booked_(false),
        timestamp_(),
        verbosity_(0),
        onlineMode_(false),
        willConvertToEDM_(true),
        edso_() {}

  DQWorker::~DQWorker() noexcept(false) {}

  /*static*/
  void DQWorker::fillDescriptions(edm::ParameterSetDescription &_desc) {
    _desc.addUntracked<bool>("onlineMode", false);
    _desc.addUntracked<bool>("willConvertToEDM", true);

    edm::ParameterSetDescription meParameters;
    edm::ParameterSetDescription meNodeParameters;
    fillMESetDescriptions(meNodeParameters);
    meParameters.addNode(
        edm::ParameterWildcard<edm::ParameterSetDescription>("*", edm::RequireZeroOrMore, false, meNodeParameters));
    _desc.addUntracked("MEs", meParameters);

    edm::ParameterSetDescription workerParameters;
    workerParameters.setUnknown();
    _desc.addUntracked("params", workerParameters);
  }

  void DQWorker::initialize(std::string const &_name, edm::ParameterSet const &_commonParams) {
    name_ = _name;
    onlineMode_ = _commonParams.getUntrackedParameter<bool>("onlineMode");
    willConvertToEDM_ = _commonParams.getUntrackedParameter<bool>("willConvertToEDM");
  }

  void DQWorker::setME(edm::ParameterSet const &_meParams) {
    std::vector<std::string> const &MENames(_meParams.getParameterNames());

    for (unsigned iME(0); iME != MENames.size(); iME++) {
      std::string name(MENames[iME]);
      edm::ParameterSet const &params(_meParams.getUntrackedParameterSet(name));

      if (!onlineMode_ && params.getUntrackedParameter<bool>("online"))
        continue;

      try {
        MEs_.insert(name, createMESet(params));
      } catch (std::exception &) {
        edm::LogError("EcalDQM") << "Exception caught while constructing MESet " << name;
        throw;
      }
    }
  }

  void DQWorker::releaseMEs() {
    for (MESetCollection::iterator mItr(MEs_.begin()); mItr != MEs_.end(); ++mItr)
      mItr->second->clear();
    booked_ = false;
  }

  void DQWorker::bookMEs(DQMStore::IBooker &_booker) {
    if (booked_)
      return;
    for (MESetCollection::iterator mItr(MEs_.begin()); mItr != MEs_.end(); ++mItr)
      mItr->second->book(_booker, GetElectronicsMap());
    booked_ = true;
  }

  void DQWorker::setSetupObjects(edm::EventSetup const &_es) {
    edm::ESHandle<EcalElectronicsMapping> elecMapHandle;
    _es.get<EcalMappingRcd>().get(elecMapHandle);
    edso_.electronicsMap = elecMapHandle.product();

    edm::ESHandle<EcalTrigTowerConstituentsMap> ttMapHandle;
    _es.get<IdealGeometryRecord>().get(ttMapHandle);
    edso_.trigtowerMap = ttMapHandle.product();

    edm::ESHandle<CaloGeometry> geomHandle;
    _es.get<CaloGeometryRecord>().get(geomHandle);
    edso_.geometry = geomHandle.product();

    edm::ESHandle<CaloTopology> topoHandle;
    _es.get<CaloTopologyRecord>().get(topoHandle);
    edso_.topology = topoHandle.product();
  }

  EcalElectronicsMapping const *DQWorker::GetElectronicsMap() {
    if (!edso_.electronicsMap)
      throw cms::Exception("InvalidCall") << "Electronics Mapping not initialized";
    return edso_.electronicsMap;
  }

  EcalTrigTowerConstituentsMap const *DQWorker::GetTrigTowerMap() {
    if (!edso_.trigtowerMap)
      throw cms::Exception("InvalidCall") << "TrigTowerConstituentsMap not initialized";
    return edso_.trigtowerMap;
  }

  CaloGeometry const *DQWorker::GetGeometry() {
    if (!edso_.geometry)
      throw cms::Exception("InvalidCall") << "CaloGeometry not initialized";
    return edso_.geometry;
  }

  CaloTopology const *DQWorker::GetTopology() {
    if (!edso_.topology)
      throw cms::Exception("InvalidCall") << "CaloTopology not initialized";
    return edso_.topology;
  }

  EcalDQMSetupObjects const DQWorker::getEcalDQMSetupObjects() {
    if (!edso_.electronicsMap)
      throw cms::Exception("InvalidCall") << "Electronics Mapping not initialized";
    if (!edso_.trigtowerMap)
      throw cms::Exception("InvalidCall") << "TrigTowerConstituentsMap not initialized";
    if (!edso_.geometry)
      throw cms::Exception("InvalidCall") << "CaloGeometry not initialized";
    if (!edso_.topology)
      throw cms::Exception("InvalidCall") << "CaloTopology not initialized";
    return edso_;
  }

  void DQWorker::print_(std::string const &_message, int _threshold /* = 0*/) const {
    if (verbosity_ > _threshold)
      edm::LogInfo("EcalDQM") << name_ << ": " << _message;
  }

  DQWorker *WorkerFactoryStore::getWorker(std::string const &_name,
                                          int _verbosity,
                                          edm::ParameterSet const &_commonParams,
                                          edm::ParameterSet const &_workerParams) const {
    DQWorker *worker(workerFactories_.at(_name)());
    worker->setVerbosity(_verbosity);
    worker->initialize(_name, _commonParams);
    worker->setME(_workerParams.getUntrackedParameterSet("MEs"));
    if (_workerParams.existsAs<edm::ParameterSet>("sources", false))
      worker->setSource(_workerParams.getUntrackedParameterSet("sources"));
    if (_workerParams.existsAs<edm::ParameterSet>("params", false))
      worker->setParams(_workerParams.getUntrackedParameterSet("params"));
    return worker;
  }

  /*static*/
  WorkerFactoryStore *WorkerFactoryStore::singleton() {
    static WorkerFactoryStore workerFactoryStore;
    return &workerFactoryStore;
  }

}  // namespace ecaldqm
