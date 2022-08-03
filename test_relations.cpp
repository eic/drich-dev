#include <cstdlib>
#include <iostream>

// ROOT
#include "TSystem.h"
#include "TFile.h"
#include "TStyle.h"
#include "TRegexp.h"
#include "TCanvas.h"

// podio
#include "podio/EventStore.h"
#include "podio/ROOTWriter.h"
#include "podio/ROOTReader.h"

// edm4hep
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char** argv) {

  // open simulation output file (from simulate.py / npsim / ddsim)
  TString infileN="sim_rich_run.root";
  if(argc>1) infileN = TString(argv[1]);
  podio::EventStore store;
  podio::ROOTReader reader;
  reader.openFile(infileN.Data());
  store.setReader(&reader);

  // setup output file
  // TString outfileN = infileN;
  // outfileN(TRegexp("\\.root$"))=".";
  // TFile *outfile = new TFile(outfileN+"plots.root","RECREATE");
  // gStyle->SetOptStat(0);

  // containers
  std::set<edm4hep::MCParticle> true_tracks;

  // event loop
  for(unsigned e=0; e<reader.getEntries(); e++) {
    cout << "\n\n\n==== EVENT " << e << " =====================================" << endl;

    // get the dRICH hits
    auto& hits = store.get<edm4hep::SimTrackerHitCollection>("DRICHHits");
    if(!hits.isValid()) continue;
    // cout << "\nDRICHHits:\n" << hits << endl; // print them

    // dRICH hits loop
    for(const auto& hit : hits) {
      cout << "  drich hit position = (" << hit.x() << ", " << hit.y() << ", " << hit.z() << ")" << endl;
      true_tracks.insert(hit.getMCParticle()); // OneToOneRelation: get the true MCParticle associated with this hit
    }

    // associated true tracks loop
    for(auto true_track : true_tracks) {
      cout << "\nTRUE TRACK\n" << true_track << endl;

      // loop over track daughters (none of which are opticalphotons, even with `ddsim --part.keepAllParticles True`)
      cout << " --> DAUGHTERS:" << endl;
      for(const auto& true_track_daughter : true_track.getDaughters() /* this is a OneToManyRelation */ ) {
        cout << "      pid = " << true_track_daughter.getPDG() << endl;
      }
    }

    // next event
    store.clear();
    true_tracks.clear();
    reader.endOfEvent();
  }
  reader.closeFile();
}
