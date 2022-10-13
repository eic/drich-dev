/* example showing how to use PODIO to read a simulation output file
 * - run `simulate.py -t1 -s` to produce a sample simulation ROOT file
 * - then run this example (see README.md for building and running)
*/

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

// local
#include "WhichRICH.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char** argv) {

  // args, figure out which RICH you are reading
  TString infileN="out/sim.root";
  if(argc<=1) {
    fmt::print("\nUSAGE: {} [d/p] [simulation_output_file(optional)]\n\n",argv[0]);
    fmt::print("    [d/p]: d for dRICH\n");
    fmt::print("           p for pfRICH\n");
    fmt::print("    [simulation_output_file]: output from `npsim` (`simulate.py`)\n");
    fmt::print("                              default: {}\n",infileN);
    return 2;
  }
  std::string zDirectionStr = argv[1];
  if(argc>2) infileN = TString(argv[2]);
  WhichRICH wr(zDirectionStr);
  if(!wr.valid) return 1;

  // open simulation output file (from simulate.py / npsim / ddsim)
  podio::EventStore store;
  podio::ROOTReader reader;
  reader.openFile(infileN.Data());
  store.setReader(&reader);

  // containers
  std::set<edm4hep::MCParticle> true_tracks;

  // event loop
  for(unsigned e=0; e<reader.getEntries(); e++) {
    cout << "\n\n\n==== EVENT " << e << " =====================================" << endl;

    // get the RICH hits
    auto& hits = store.get<edm4hep::SimTrackerHitCollection>(wr.XRICH+"Hits");
    if(!hits.isValid()) continue;
    // cout << "\nHits:\n" << hits << endl; // print them

    // RICH hits loop
    for(const auto& hit : hits) {
      cout << "  rich hit position = (" << hit.x() << ", " << hit.y() << ", " << hit.z() << ")" << endl;
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
