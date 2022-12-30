// draw track points of projected trajectories on dRICH planes
//
struct radiator_config {
  int         id;
  std::string name;
  EColor      color;
};

void draw_tracks(TString infileN = "out/rec.root") {
  auto infile = new TFile(infileN);
  auto tr = (TTree*) infile->Get("events");
  
  std::vector<radiator_config> radiators = {
    radiator_config{ 0, "Aerogel", kRed   },
    radiator_config{ 1, "Gas",     kBlack }
  };

  double rmax = 1900;
  double xmax = rmax;
  double ymax = rmax;
  double zmin = 1800;
  double zmax = 3300;

  // ymax = 60; // zoom to horizontal (y=0) plane

  std::vector<TH3D*> histograms;
  for(auto radiator : radiators) {
    TString histName = Form("%s_hist",radiator.name.c_str());
    TString collName = Form("DRICH%sTracks_0",radiator.name.c_str());
    histograms.push_back(
        new TH3D(
          histName,
          "Propagated track points",
          // int(zmax-zmin), zmin,  zmax,
          // int(2*rmax),    -rmax, rmax,
          // int(2*rmax),    -rmax, rmax
          100, zmin,  zmax,
          100, -xmax, xmax,
          100, -ymax, ymax
          )
        );
    histograms.back()->SetMarkerColor(radiator.color);
    histograms.back()->SetMarkerStyle(kFullCircle);
    tr->Project(
        histName,
        collName+".position.y:"+collName+".position.x:"+collName+".position.z"
        );
  }

  auto canv = new TCanvas();
  for(auto histogram : histograms) histogram->Draw("same");
}

