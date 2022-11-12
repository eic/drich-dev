// tests reading simulation/reconstruction PODIO output using
// podio::EventStore
//

R__LOAD_LIBRARY(podio)
R__LOAD_LIBRARY(podioDict)
R__LOAD_LIBRARY(podioRootIO)
R__LOAD_LIBRARY(edm4hep)
R__LOAD_LIBRARY(edm4eic)
R__LOAD_LIBRARY(edm4hepDict)
R__LOAD_LIBRARY(edm4eicDict)
R__LOAD_LIBRARY(fmt)
#include "podio/EventStore.h"
#include "podio/ROOTReader.h"
#include "podio/CollectionBase.h"
#include "fmt/format.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"

void test_EventStore_read(
    int testNum=1,
    const char *root_file_name = "out/2022-11-10_pgun_pi-_epic_arches_e0.01-30GeV_alldir_1prt_10000evt.tree.edm4eic.root"
    // const char *root_file_name = "out/rec_full_arches.juggler.tree.edm4eic.root"
    // const char *root_file_name = "out/rec_full_arches.eicrecon.tree.edm4eic.root"
    )
{
  podio::ROOTReader reader;
  podio::EventStore store;
  reader.openFile(root_file_name);
  store.setReader(&reader);

  if(testNum==0) { // `get` the collections from the 1st event, and print their sizes
    auto &simParts = store.get<edm4hep::MCParticleCollection>("MCParticles");
    auto &recParts = store.get<edm4eic::ReconstructedParticleCollection>("ReconstructedChargedParticles");
    fmt::print("simParts.size={}\n",simParts.size());
    fmt::print("recParts.size={}\n",recParts.size());
  }

  else if(testNum==1) { // `get` the association collection from the 1st event
    auto &assocs = store.get<edm4eic::MCRecoParticleAssociationCollection>("ReconstructedChargedParticlesAssociations");
    fmt::print("assocs.size={}\n",assocs.size());

  }

  else if(testNum==2) { // print the collection and data types
    std::vector<std::string> collNames = {
      "MCParticles"
        , "ReconstructedChargedParticles"
        , "ReconstructedChargedParticlesAssociations"
    };
    for(auto collName : collNames) {
      const auto collID = store.getCollectionIDTable()->collectionID(collName);
      podio::CollectionBase *collBase;
      if(store.get(collID, collBase))
        fmt::print("\ndata types for collection '{}'\n{:>50}\n{:>50}\n{:>50}\n",
            collName,
            collBase->getTypeName(),
            collBase->getValueTypeName(),
            collBase->getDataTypeName()
            );
    }
  }

  else if(testNum==3) { // try to access collections in an event loop
    fmt::print("begin event loop...\n");
    for(unsigned e=0; e<reader.getEntries(); e++) {
      // access collections, and print their sizes
      fmt::print("Event {}:",e);
      auto &simParts = store.get<edm4hep::MCParticleCollection>("MCParticles"); fmt::print(" simParts.size={}",simParts.size());
      auto &recParts = store.get<edm4eic::ReconstructedParticleCollection>("ReconstructedChargedParticles"); fmt::print(" recParts.size={}",recParts.size());
      fmt::print("\n");
      // next event
      store.clear();
      reader.endOfEvent();
    }
  }
}
