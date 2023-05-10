// test if the pixel gap cuts are working
// 1. add member `edm4hep::Vector3d position` to `edm4eic::RawPMTHit`
// 2. fill this member in `PhotoMultiplierHitDigi.cc`
// 3. run a simulation, with varying momentum directions, then run reconstruction
// 4. run this script on the output

R__LOAD_LIBRARY(fmt)
#include <fmt/core.h>

void test_pixel_gap_cuts() {

  auto infile = new TFile("out/rec.root");
  auto tr = (TTree*) infile->Get("events");

  // draw gaps in x,y
  if(tr->GetBranch("DRICHRawHits.position.x")!=nullptr) {
    auto c = new TCanvas();
    auto h = new TH2D("h","h",10000,-15,15,10000,-15,15);
    tr->Project("h","DRICHRawHits.position.y:DRICHRawHits.position.x","");
    fmt::print("ENTRIES = {}\n",h->GetEntries());
    h->Draw();
    c->SaveAs("out/gap.png");
  }
}
