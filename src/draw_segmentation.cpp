// test readout segmentation
#include <cstdlib>
#include <iostream>
#include <bitset>
#include <map>
#include <vector>
#include <fmt/format.h>

// ROOT
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TBox.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

// DD4Hep
#include "DD4hep/Detector.h"

using namespace ROOT;
using namespace dd4hep;

int main(int argc, char** argv) {

  // arguments
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  TString infileN="out/sim.root";
  if(argc>1) infileN = TString(argv[1]);

  // settings
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // number of pixels along sensor side
  const Int_t numPx = 8;

  // dilations: for re-scaling module positions and segment positions
  // for drawing; if you change `numPx`, consider tuning these parameters
  // as well
  const Int_t dilation = 5;

  // drawing
  gStyle->SetPalette(55);
  gStyle->SetOptStat(0);
  const Bool_t singleCanvas = false; // if true, draw all hitmaps on one canvas


  // setup
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // define application environment, to keep canvases open
  TApplication mainApp("mainApp",&argc,argv);

  // main dataframe
  RDataFrame dfIn("events",infileN.Data());

  // compact file name
  std::string DETECTOR_PATH(getenv("DETECTOR_PATH"));
  std::string DETECTOR(getenv("DETECTOR"));
  if(DETECTOR_PATH.empty() || DETECTOR.empty()) {
    fmt::print(stderr,"ERROR: source environ.sh\n");
    return 1;
  }
  std::string compactFile = DETECTOR_PATH + "/" + DETECTOR + ".xml";

  // get detector handle and some constants
  const std::string richName    = "DRICH";
  const std::string readoutName = "DRICHHits";
  const auto det = &(Detector::getInstance());
  det->fromXML(compactFile);
  const auto detRich  = det->detector(richName);
  const auto posRich  = detRich.placement().position();
  const auto cellMask = ULong_t(std::stoull(det->constant<std::string>("DRICH_RECON_cellMask")));
  const auto nSectors = det->constant<int>("DRICH_RECON_nSectors");

  // cellID decoder
  /* - `decodeCellID(fieldName)` returns a "decoder" for the field with name `fieldName`
   * - this decoder is a function that maps an `RVecUL` of `cellID`s to an
   *   `RVecUL` of correpsonding field element values
   */
  const auto readoutCoder = det->readout(readoutName).idSpec().decoder();
  auto decodeCellID = [&readoutCoder] (std::string fieldName) {
    return [&readoutCoder,&fieldName] (RVecUL cellIDvec) {
      RVecUL result;
      for(const auto& cellID : cellIDvec) {
        auto val = readoutCoder->get(cellID,fieldName); // get BitFieldElement value
        result.emplace_back(val);
        // fmt::print("decode {}: {:64b} -> {}\n",fieldName,cellID,val);
      }
      return result;
    };
  };

  // build sensor position LUT `imod2hitmapXY`
  /* - find the sector 0 sensors, and build a map of their module number `imod` to
   *   X and Y coordinates to use in the hitmap
   *   - these hitmap coordinates are from the sensor position X and Y, rescaled
   *     by the factor `dilation` and rounded to the nearest integer
   *   - also builds a list of `TBox`es, for drawing the sensors on the hitmap
   * - the unique ID of the sensor `Detector`, called `imodsec`, includes `imod`
   *   - `cellID & cellMask` should be equivalent to `imodsec`; therefore,
   *     `imodsec` can be converted to `imod` by decoding `imodsec` the same way
   *     we would decode `cellID`
   */
  std::map<ULong_t,std::pair<Long64_t,Long64_t>> imod2hitmapXY;
  std::vector<TBox*> boxList;
  for(auto const& [de_name, detSensor] : detRich.children()) {
    if(de_name.find("sensor_de_sec0")!=std::string::npos) {
      // convert global position to hitmapX and Y
      auto posSensor = posRich + detSensor.placement().position();
      auto hitmapX   = Long64_t(dilation*posSensor.x() + 0.5);
      auto hitmapY   = Long64_t(dilation*posSensor.y() + 0.5);
      // convert unique cellID to module number, using the cellID decoder
      auto imodsec = ULong_t(detSensor.id());
      auto imod    = decodeCellID("module")(RVecUL({imodsec})).front();
      // add to `imod2hitmapXY` and create sensor `TBox`
      imod2hitmapXY.insert({imod,{hitmapX,hitmapY}});
      boxList.push_back(new TBox(
            hitmapX, 
            hitmapY,
            hitmapX + numPx,
            hitmapY + numPx
            ));
      boxList.back()->SetLineColor(kGray);
    }
  }

  // convert vector of `imod`s to vector of hitmap X or Y
  auto imod2hitmapXY_get = [&imod2hitmapXY] (RVecUL imodVec, int c) {
    RVecL result;
    Long64_t pos;
    for(auto imod : imodVec) {
      try {
        pos = (c==0) ?
          imod2hitmapXY[imod].first :
          imod2hitmapXY[imod].second;
      }
      catch (const std::out_of_range &ex) {
        fmt::print(stderr,"ERROR: cannot find module {}\n",imod);
        pos = 0.0;
      };
      result.emplace_back(pos);
    }
    return result;
  };
  auto imod2hitmapX = [&imod2hitmapXY_get] (RVecUL cellIDvec) { return imod2hitmapXY_get(cellIDvec,0); };
  auto imod2hitmapY = [&imod2hitmapXY_get] (RVecUL cellIDvec) { return imod2hitmapXY_get(cellIDvec,1); };

  // convert vector of hitmap X (or Y) + vector of segmentation X (or Y) to vector of pixel X (or Y)
  auto pixelCoord = [] (RVecL hitmapXvec, RVecUL segXvec) {
    RVecL result = hitmapXvec + segXvec;
    return result;
  };


  // dataframe transformations
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  auto dfOut = dfIn
      .Alias("cellID","DRICHHits.cellID")
      // decode cellID
      .Define("system", decodeCellID("system"), {"cellID"})
      .Define("sector", decodeCellID("sector"), {"cellID"})
      .Define("module", decodeCellID("module"), {"cellID"})
      .Define("x",      decodeCellID("x"),      {"cellID"})
      .Define("y",      decodeCellID("y"),      {"cellID"})
      // convert `module`s to hitmap positions
      .Define("hitmapX", imod2hitmapX, {"module"})
      .Define("hitmapY", imod2hitmapY, {"module"})
      // convert (hitmap,`iseg`) positions to hitmap positions
      .Define("pixelX", pixelCoord, {"hitmapX","x"})
      .Define("pixelY", pixelCoord, {"hitmapY","y"})
      ;


  // histograms
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // cellID field histograms
  auto fieldHists = std::vector({
    dfOut.Histo1D("system"),
    dfOut.Histo1D("sector"),
    dfOut.Histo1D("module"),
    dfOut.Histo1D("x"),
    dfOut.Histo1D("y")
  });
  const int segXmax = 10;
  auto segXY = dfOut.Histo2D(
      { "segXY", "CartesianGridXY;x;y",
        2*segXmax, -segXmax, segXmax,
        2*segXmax, -segXmax, segXmax },
      "x","y"
      );


  // pixel hitmap
  Double_t pixelXmin = dilation * 100;
  Double_t pixelXmax = dilation * 190;
  Double_t pixelYmin = dilation * -70;
  Double_t pixelYmax = dilation * 70;
  auto pixelHitmap = dfOut.Histo3D(
      { "pixelHitmap", "Pixel Hit Map;x;y;sector",
        (Int_t)(pixelXmax-pixelXmin), pixelXmin, pixelXmax,
        (Int_t)(pixelYmax-pixelYmin), pixelYmin, pixelYmax,
        nSectors,0,double(nSectors) },
      "pixelX","pixelY","sector"
      );

  // draw
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // draw cellID field histograms
  TCanvas *c = new TCanvas();
  c->Divide(3,2);
  int pad=1;
  for(auto hist : fieldHists) {
    c->GetPad(pad)->SetLogy();
    c->cd(pad);
    if(TString(hist->GetName())!="module") hist->SetBarWidth(4);
    hist->SetLineColor(kBlack);
    hist->SetFillColor(kBlack);
    hist->Draw("bar");
    pad++;
  }

  // draw segmentation XY plot, along with expected box
  c->cd(pad);
  c->GetPad(pad)->SetGrid(1,1);
  segXY->Draw("colz");
  auto expectedBox = new TBox(0,0,numPx,numPx);
  expectedBox->SetFillStyle(0);
  expectedBox->SetLineColor(kBlack);
  expectedBox->SetLineWidth(8);
  expectedBox->Draw("same");
  segXY->Draw("colz same");

  // draw pixel hitmap
  if(singleCanvas) { c = new TCanvas(); c->Divide(3,2); };
  int secBin;
  TH2D *pixelHitmapSec[nSectors];
  for(int sec=0; sec<nSectors; sec++) {
    if(singleCanvas) c->cd(sec+1); else c = new TCanvas();
    secBin = pixelHitmap->GetZaxis()->FindBin(Double_t(sec));
    pixelHitmap->GetZaxis()->SetRange(secBin,secBin);
    pixelHitmapSec[sec] = (TH2D*) pixelHitmap->Project3D("yx");
    pixelHitmapSec[sec]->SetName(Form("pixelHitmap_s%d",sec));
    pixelHitmapSec[sec]->SetTitle(Form("pixel hits sector %d",sec));
    pixelHitmapSec[sec]->Draw("colz");
    for(auto box : boxList) {
      box->SetFillStyle(0);
      box->Draw("same");
    };
    pixelHitmapSec[sec]->Draw("colz same");
  };

  fmt::print("\n\npress ^C to exit.\n\n");
  mainApp.Run();
  return 0;
};
