// draw track points of projected trajectories on dRICH planes
//


class radiator_config {
  public:
    radiator_config(int id_, std::string name_, Color_t color_, Style_t style_, TTreeReader& reader) :
      id(id_),
      name(name_),
      color(color_),
      style(style_),
      collName(Form("DRICH%s_0",name.c_str())),
      x_arr({reader, collName+".position.x"}),
      y_arr({reader, collName+".position.y"}),
      z_arr({reader, collName+".position.z"}) {}
    ~radiator_config() {}
    int         id;
    std::string name;
    TString     collName;
    Color_t     color;
    Style_t     style;
    TTreeReaderArray<Float_t> x_arr, y_arr, z_arr;
};



void test_tracks(TString infileN = "out/rec.edm4hep.root") {
  auto infile = new TFile(infileN);
  auto tr = (TTree*) infile->Get("events");
  TTreeReader tr_reader("events", infile);
  
  std::vector<radiator_config*> radiators = {
    new radiator_config( 0, "AerogelTracks",       kAzure+7, kFullCircle, tr_reader ),
    new radiator_config( 1, "GasTracks",           kRed,     kFullCircle, tr_reader ),
    // new radiator_config( 0, "AerogelPseudoTracks", kGreen+2, kOpenCircle, tr_reader ),
    // new radiator_config( 1, "GasPseudoTracks",     kMagenta, kOpenCircle, tr_reader )
  };

  double rmax = 1900;
  double xmax = rmax;
  double ymax = rmax;
  double zmin = 1800;
  double zmax = 3300;

  ymax = 200; // zoom to horizontal (y=0) plane

  // 2D plots
  enum views { zx, zy, xy, Nview };
  TString viewN[Nview], viewT[Nview];
  viewN[zx] = "zx";
  viewN[zy] = "zy";
  viewN[xy] = "xy";
  viewT[zx] = "Top View (toward -y);z [mm];x [mm]";
  viewT[zy] = "Side View (toward +x);z [mm];y [mm]";
  viewT[xy] = "Front View (toward +z);x [mm];y [mm]";
  std::vector<TGraph*> points2[Nview];
  TMultiGraph *points2mgr[Nview];
  for(int view=0; view<Nview; view++) {
    points2mgr[view] = new TMultiGraph();
    points2mgr[view]->SetName("points_"+viewN[view]);
    points2mgr[view]->SetTitle(viewT[view]);
  }
  for(auto& radiator : radiators) {
    for(int view=0; view<Nview; view++) {
      auto gr = new TGraph();
      points2[view].push_back(gr);
      points2mgr[view]->Add(gr);
      gr->SetMarkerColor(radiator->color);
      gr->SetMarkerStyle(radiator->style);
      gr->SetName(Form("points_%s_%s",viewN[view].Data(),radiator->name.c_str()));
      gr->SetTitle(viewT[view]);
      tr_reader.Restart();
      while(tr_reader.Next()) {
        auto i_offset = gr->GetN();
        for(int i=0; i<radiator->x_arr.GetSize(); i++) {
          auto x = radiator->x_arr[i];
          auto y = radiator->y_arr[i];
          auto z = radiator->z_arr[i];
          switch(view) {
            case zx: gr->SetPoint(i+i_offset,z,x); break;
            case zy: gr->SetPoint(i+i_offset,z,y); break;
            case xy: gr->SetPoint(i+i_offset,x,y); break;
          }
        }
      }
    }
  }
  auto canv2 = new TCanvas("canv2","canv2",Nview*800,600);
  canv2->Divide(Nview,1);
  for(int view=0; view<Nview; view++) {
    canv2->cd(view+1);
    canv2->GetPad(view+1)->SetGrid(0,1);
    canv2->GetPad(view+1)->SetLeftMargin(0.15);
    TString mode = "AP";
    if(view==xy) mode+=" RX";
    points2mgr[view]->Draw(mode);
  }
  points2mgr[zx]->GetXaxis()->SetLimits(     zmin, zmax );
  points2mgr[zx]->GetYaxis()->SetRangeUser( -xmax, xmax );
  points2mgr[zy]->GetXaxis()->SetLimits(     zmin, zmax );
  points2mgr[zy]->GetYaxis()->SetRangeUser( -ymax, ymax );
  points2mgr[xy]->GetXaxis()->SetLimits(    -xmax, xmax );
  points2mgr[xy]->GetYaxis()->SetRangeUser( -ymax, ymax );
  canv2->SaveAs("out/tracks.png");

  // 3D plots (NOTE: point positions inaccurate, since 3D histogram bins)
  std::vector<TH3D*> points3;
  for(auto& radiator : radiators) {
    TString histName = Form("%s_hist",radiator->name.c_str());
    points3.push_back(
        new TH3D(
          histName,
          "Propagated track points",
          100, zmin,  zmax,
          100, -xmax, xmax,
          100, -ymax, ymax
          )
        );
    points3.back()->SetMarkerColor(radiator->color);
    points3.back()->SetMarkerStyle(kFullCircle);
    tr->Project(
        histName,
        radiator->collName+".position.y:"+radiator->collName+".position.x:"+radiator->collName+".position.z"
        );
  }
  auto canv3 = new TCanvas();
  for(auto plot : points3) plot->Draw("same");
}

