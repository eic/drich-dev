// draw various momentum scan plots
R__LOAD_LIBRARY(podioDict)
R__LOAD_LIBRARY(podioRootIO)
R__LOAD_LIBRARY(edm4hep)
R__LOAD_LIBRARY(edm4eic)
R__LOAD_LIBRARY(fmt)

#include "fmt/format.h"

// radiators
enum rad_enum {kAgl, kGas, nRad};

//=========================================================

void momentum_scan_eicrecon_draw(
    TString  ana_file_name  = "out/rec.ana.root",        // output analysis file from EICrecon benchmarks
    TString  out_file_name  = "out/rec.scan_plots.root", // this script will produce this file
    unsigned which_radiator = 1,                         // see `rad_enum` above
    TString  ana_dir_name   = "pid_irt"                  // which directory in `ana_file_name` file
    )
{

  // files
  TFile *ana_file = new TFile(TString(ana_file_name));
  TFile *out_file = new TFile(TString(out_file_name),"RECREATE");

  // radiators
  TString radiator_name;
  double mom_max;
  switch(which_radiator) {
    case kAgl:
      radiator_name = "Aerogel";
      mom_max       = 22;
      break;
    case kGas:
      radiator_name = "Gas";
      mom_max       = 65;
      break;
    default:
      fmt::print(stderr,"ERROR: unknown which_radiator={}\n",which_radiator);
      return;
  }

  // get plots
  std::map<TString,TH2D*> scans;
  auto get_scan = [&] (TString name) {
    TString hist_name = ana_dir_name + "/" + name + "_vs_p_" + radiator_name;
    fmt::print("get histogram '{}'\n",hist_name);
    auto hist = ana_file->Get<TH2D>(hist_name);
    hist->SetName(name+"_scan");
    scans.insert({name,hist});
  };
  get_scan("npe");
  get_scan("theta");

  // make scan profiles
  std::map<TString,TProfile*> scan_profs;
  std::map<TString,TCanvas*> scan_canvs;
  for(const auto& [name,scan] : scans) {
    auto prof = scan->ProfileX("_pfx",1,-1,"");
    prof->SetLineColor(kBlack);
    prof->SetLineWidth(3);
    scan_profs.insert({name,prof});
    TString canv_name(name+"_canv");
    auto canv = new TCanvas(canv_name,canv_name);
    canv->SetGrid(1,1);
    scan->Draw("box");
    prof->Draw("same");
    scan_canvs.insert({name,canv});
  }

  // write output
  auto WriteScans = [] (auto vec) { for(const auto& [name,obj] : vec) obj->Write(); };
  WriteScans(scans);
  WriteScans(scan_profs);
  WriteScans(scan_canvs);
  out_file->Close();
  fmt::print("\n{:=<50}\nwrote: {}\n\n","",out_file_name);
}
