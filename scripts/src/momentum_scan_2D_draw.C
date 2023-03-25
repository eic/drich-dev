// draw combined theta vs. p plot
R__LOAD_LIBRARY(fmt)
#include "fmt/format.h"

// radiators
enum rad_enum {kAgl, kGas, nRad};

//=========================================================

void momentum_scan_2D_draw(
    TString  ana_file_names = "out/momentum_scan.drich/*.rec.agl.ana.root", // input files (will be hadded)
    TString  out_base_name  = "out/2D",                                     // output; should be png or pdf
    unsigned which_radiator = 0                                             // see `rad_enum` above
    )
{

  // hadd input files
  TString hadd_file_name = out_base_name + ".root";
  gROOT->ProcessLine(".! hadd -f " + hadd_file_name + " " + ana_file_names);
  auto hadd_file = new TFile(hadd_file_name,"UPDATE");

  // radiators
  TString radiator_name;
  double mom_max, theta_max, rindex_ref;
  switch(which_radiator) {
    case kAgl:
      radiator_name = "Aerogel";
      mom_max       = 22;  // for plot zoom
      theta_max     = 230; // for plot zoom
      rindex_ref    = 1.0190;
      break;
    case kGas:
      radiator_name = "Gas";
      mom_max       = 65; // for plot zoom
      theta_max     = 50; // for plot zoom
      rindex_ref    = 1.00076;
      break;
    default:
      fmt::print(stderr,"ERROR: unknown which_radiator={}\n",which_radiator);
      return;
  }

  // theta curves
  std::map<TString,double> particle_masses = {
    { "e-",     0.00051 },
    { "pi+",    0.13957 },
    { "kaon+",  0.49368 },
    { "proton", 0.93827 }
  };
  std::map<TString,TF1*> theta_curves;
  for(auto [particle,mass] : particle_masses)
    theta_curves.insert({
        particle,
        new TF1(
            "ftn_theta_" + particle,
            Form("1000*TMath::ACos(TMath::Sqrt(x^2+%f^2)/(%f*x))", mass, rindex_ref),
            mass / TMath::Sqrt(rindex_ref * rindex_ref - 1),
            mom_max
          )
        });

  // get plots
  std::map<TString,TH2D*> scans;
  auto get_scan = [&] (TString dir, TString name, TString suffix="") {
    TString hist_name = dir + "/" + name + "_vs_p";
    if(suffix!="") hist_name += "_" + suffix;
    fmt::print("get histogram '{}'\n",hist_name);
    auto hist = hadd_file->Get<TH2D>(hist_name);
    hist->SetName(name+"_scan");
    scans.insert({name,hist});
  };
  get_scan( "pid_irt", "theta", radiator_name );

  // draw
  gStyle->SetOptStat(0);
  std::map<TString,TCanvas*> scan_canvs;
  for(const auto& [name,scan] : scans) {
    TString canv_name(name+"_canv");
    auto canv = new TCanvas(canv_name,canv_name);
    canv->SetGrid(1,1);
    scan->Draw("COLZ");
    scan->GetXaxis()->SetRangeUser(0,mom_max);
    scan->GetYaxis()->SetRangeUser(0,theta_max);
    for(auto [particle,curve] : theta_curves) {
      curve->Draw("SAME");
    }
    canv->SaveAs(out_base_name + "_" + name + ".png");
    scan_canvs.insert({name,canv});
  }

  // write output
  auto WriteScans = [] (auto vec) { for(const auto& [name,obj] : vec) obj->Write(); };
  WriteScans(scans);
  WriteScans(scan_canvs);
  hadd_file->Close();
  fmt::print("\n{:=<50}\nwrote: {}\n\n","",hadd_file_name);
}
