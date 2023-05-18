// draw various momentum scan plots
R__LOAD_LIBRARY(podioDict)
R__LOAD_LIBRARY(podioRootIO)
R__LOAD_LIBRARY(edm4hep)
R__LOAD_LIBRARY(edm4eic)
R__LOAD_LIBRARY(fmt)

#include "podio/EventStore.h"
#include "podio/ROOTReader.h"
#include "edm4hep/utils/kinematics.h"
#include "fmt/format.h"

// radiators
enum rad_enum {kAgl=0, kGas=1};

//=========================================================

void momentum_scan_juggler_draw(
    std::string rec_file_name  = "out/rec.edm4hep.root",
    std::string out_file_name  = "out/rec.scan_plots.root",
    std::string det_name       = "DRICH", // or PFRICH
    unsigned    which_radiator = 0 // see `rad_enum` above
    )
{
  // open event store
  podio::ROOTReader reader;
  podio::EventStore store;
  reader.openFile(rec_file_name);
  store.setReader(&reader);

  // output root file
  TFile *out_file = new TFile(TString(out_file_name),"RECREATE");

  // radiators
  std::string radiator_name;
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

  // plot limits
  const double theta_max = 260.0;
  const int    nphot_max = 500;
  const int    npe_max   = 0.3*nphot_max;

  // define histograms
  std::map<std::string,TH2D*> scans;
  auto make_scan = [&scans,&mom_max,&radiator_name] (
      std::string name,
      std::string title,
      std::string units,
      auto yn, auto yi, auto yf
      )
  {
    if(units!="") units=" ["+units+"]";
    std::string for_radiator = (name!="nphot") ? ", for "+radiator_name : "";
    auto hist = new TH2D(
        TString(name + "_scan"),
        TString(title + " vs. Thrown Momentum" + for_radiator +";p [GeV];" + title + units),
        (int)mom_max+1, 0, mom_max,
        yn, yi, yf
        );
    hist->StatOverflows(true);
    scans.insert({name,hist});
  };
  make_scan( "nphot", "N_{photons}",        "",     nphot_max, 0, nphot_max );
  make_scan( "npe",   "N_{photoelectrons}", "",     npe_max,   0, npe_max   );
  make_scan( "theta", "#theta_{Cherenkov}", "mrad", 100,       0, theta_max );

  // function to Fill a scan: warns for under/overflows
  auto FillScan = [&scans] (std::string name, auto x, auto y) {
    auto check = [] (auto val, std::string ax_name, TAxis *ax) {
      if(val<ax->GetXmin() || val>ax->GetXmax())
        fmt::print(stderr,"WARNING: {} overflow: {}\n",ax_name,val);
    };
    auto scan = scans.at(name);
    check(x, name+" scan x-axis", scan->GetXaxis());
    check(y, name+" scan y-axis", scan->GetYaxis());
    scan->Fill(x,y);
  };

  // event loop =================================================================
  for(unsigned e=0; e<reader.getEntries(); e++) {

    // get next event
    // fmt::print("read event #{}\n",e);
    if(e>0) {
      store.clear();
      reader.endOfEvent();
    };

    // get collections
    const auto& mcParts = store.get<edm4hep::MCParticleCollection>("MCParticles");
    const auto& hits    = store.get<edm4hep::SimTrackerHitCollection>(det_name+"Hits");
    const auto& cpids   = store.get<edm4eic::CherenkovParticleIDCollection>(det_name+"PID");

    // thrown momentum
    int nthrown = 0;
    float mom;
    for(const auto& mcPart : mcParts) {
      if(mcPart.getGeneratorStatus()==1) {
        mom = edm4hep::utils::p(mcPart);
        nthrown++;
      }
    }
    if(nthrown == 0) { fmt::print(stderr,"WARNING: no thrown particle found for event #{}\n",e); continue; };
    if(nthrown >  1)   fmt::print(stderr,"WARNING: this script does not yet support multi-track events (nthrown={})\n",nthrown);

    // number of photons
    FillScan("nphot", mom, hits.size());

    // NPE, Cherenkov angle
    if(cpids.size() != 1) fmt::print(stderr,"WARNING: CherenkovParticleIDCollection size = {} != 1\n",cpids.size());
    for(const auto& cpid : cpids) {
      for(const auto &meas : cpid.getAngles()) {
        auto npe = meas.npe;
        auto theta = meas.theta * 1000; // [mrad]
        bool pass =
          meas.radiator == which_radiator
          && npe>0
          ;
        if(pass) {
          FillScan( "npe",   mom, npe   );
          FillScan( "theta", mom, theta );
        }
      }
    }

  } // end event loop =================================================================
  store.clear();
  reader.endOfEvent();
  reader.closeFile();

  // make scan profiles
  std::map<std::string,TProfile*> scan_profs;
  std::map<std::string,TCanvas*> scan_canvs;
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
