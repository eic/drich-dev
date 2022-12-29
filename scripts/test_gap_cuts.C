// test if the pixel gap cuts are working
// usage:
// 1. add member `edm4hep::Vector3d position` to `edm4eic::RawPMTHit`
// 2. fill this member in `PhotoMultiplierHitDigi.cc`
// 3. run a simulation, with varying momentum directions, then run reconstruction
// 4. run this script on the output

void test_gap_cuts() {

  // draw gaps in x,y
  auto infile = new TFile("out/rec.root");
  auto tr = (TTree*) infile->Get("events");
  auto c = new TCanvas();
  auto h = new TH2D("h","h",10000,-15,15,10000,-15,15);
  tr->Project("h","DRICHRawHits.position.y:DRICHRawHits.position.x","");
  cout << "ENTRIES = " << h->GetEntries() << endl;
  h->Draw();
  c->SaveAs("out/gap.png");

  /*
  auto infile = new TFile("out/rec.root");
  auto tr = (TTree*) infile->Get("events");
  auto h = new TH1D("h","h",1000,0,10000);
  tr->Project("h","DRICHRawHits.integral");
  cout << "ENTRIES = " << h->GetEntries() << endl;
  */


}
