// test readout segmentation
#include <cstdlib>
#include <iostream>
#include <bitset>
#include <map>
#include <vector>
#include <algorithm>
#include <fmt/format.h>

// ROOT
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TBox.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RVec.hxx"

// DD4Hep
#include "DD4hep/Detector.h"

// local
#include "WhichRICH.h"

using namespace ROOT;
using namespace dd4hep;

int main(int argc, char** argv) {

  // arguments
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if(argc<=3) {
    fmt::print("\nUSAGE: {} [d/p] [s/r] [input_root_file] [event_number (optional)]\n\n",argv[0]);
    fmt::print("    [d/p]: detector\n");
    fmt::print("         - d for dRICH\n");
    fmt::print("         - p for pfRICH\n");
    fmt::print("\n");
    fmt::print("    [s/r]: file type:\n");
    fmt::print("         - s for simulation file (all photons)\n");
    fmt::print("         - r for reconstructed file (digitized hits)\n");
    fmt::print("\n");
    fmt::print("    [input_root_file]: output from simulation or reconstruction\n");
    fmt::print("\n");
    fmt::print("    [event_number]: if specified, draw a single event\n");
    fmt::print("                    otherwise the sum of all events is drawn\n");
    fmt::print("\n");
    return 2;
  }
  std::string zDirectionStr = argv[1];
  std::string fileType      = argv[2];
  TString     infileN       = TString(argv[3]);
  int         evnumArg      = argc>4 ? std::atoi(argv[4]) : -1;
  WhichRICH wr(zDirectionStr);
  if(!wr.valid) return 1;

  // settings
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // number of pixels along sensor side
  const Int_t numPx = 8;

  // expected range of values of `x` and `y` `cellID` bit fields
  const Int_t segXmin = 0;
  const Int_t segXmax = numPx-1;

  // dilations: for re-scaling module positions and segment positions
  // for drawing; if you change `numPx`, consider tuning these parameters
  // as well
  const Int_t dilation = 4;

  // drawing
  gStyle->SetPalette(55);
  gStyle->SetOptStat(0);
  const Bool_t singleCanvas = true; // if true, draw all hitmaps on one canvas

  // data collections
  std::string inputCollection;
  if(fileType=="s")
    inputCollection = wr.readoutName;
  else if(fileType=="r")
    inputCollection = wr.rawHitsName;
  else {
    fmt::print(stderr,"ERROR: unknown file type '{}'\n",fileType);
    return 1;
  }
  fmt::print("Reading collection '{}'\n",inputCollection);
  if(evnumArg<0) fmt::print("Reading all events\n");
  else fmt::print("Reading only event number {}\n",evnumArg);


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
  const std::string richName = wr.XRICH;
  const auto det = &(Detector::getInstance());
  det->fromXML(compactFile);
  const auto detRich  = det->detector(richName);
  const auto posRich  = detRich.placement().position();
  const auto cellMask = ULong_t(std::stoull(det->constant<std::string>(wr.XRICH+"_cell_mask")));
  const auto nSectors = wr.zDirection>0 ? det->constant<int>(wr.XRICH+"_num_sectors") : 1;

  // cellID decoder
  /* - `decodeCellID(fieldName)` returns a "decoder" for the field with name `fieldName`
   * - this decoder is a function that maps an `RVecUL` of `cellID`s to an
   *   `RVecUL` of correpsonding field element values
   */
  const auto readoutCoder = det->readout(wr.readoutName).idSpec().decoder();
  auto decodeCellID = [&readoutCoder] (std::string fieldName) {
    return [&readoutCoder,&fieldName] (RVecUL cellIDvec) {
      RVecL result;
      bool found=false;
      for(auto elem : readoutCoder->fields())
        if(fieldName == elem.name()) { found = true; break; }
      // if(!found) fmt::print("- skipping missing bit field \"{}\"\n",fieldName);
      for(const auto& cellID : cellIDvec) {
        if(found) {
          auto val = readoutCoder->get(cellID,fieldName); // get BitFieldElement value
          result.emplace_back(val);
          // fmt::print("decode {}: {:64b} -> {}\n",fieldName,cellID,val);
        } else result.emplace_back(0);
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
  std::map<Long_t,std::pair<Long64_t,Long64_t>> imod2hitmapXY;
  std::vector<TBox*> boxList;
  for(auto const& [de_name, detSensor] : detRich.children()) {
    if(de_name.find(wr.sensorNamePattern)!=std::string::npos) {
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
      boxList.back()->SetLineColor(kGreen-10);
      boxList.back()->SetFillColor(kGreen-10);
      boxList.back()->SetFillStyle(1001);
    }
  }

  // convert vector of `imod`s to vector of hitmap X or Y
  auto imod2hitmapXY_get = [&imod2hitmapXY] (RVecL imodVec, int c) {
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
  auto imod2hitmapX = [&imod2hitmapXY_get] (RVecL modVec) { return imod2hitmapXY_get(modVec,0); };
  auto imod2hitmapY = [&imod2hitmapXY_get] (RVecL modVec) { return imod2hitmapXY_get(modVec,1); };

  // convert vector of hitmap X (or Y) + vector of segmentation X (or Y) to vector of pixel X (or Y)
  auto pixelCoord = [] (RVecL hitmapXvec, RVecL segXvec) {
    RVecL result = hitmapXvec + segXvec;
    return result;
  };


  // dataframe transformations
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // decode cellID to bit field element values
  auto dfDecoded = dfIn
      .Range(
          evnumArg<0 ? 0 : evnumArg,
          evnumArg<0 ? 0 : evnumArg+1
          )
      .Alias("cellID", inputCollection+".cellID")
      .Define("system", decodeCellID("system"), {"cellID"})
      .Define("sector", decodeCellID("sector"), {"cellID"})
      .Define("module", decodeCellID("module"), {"cellID"})
      .Define("x",      decodeCellID("x"),      {"cellID"})
      .Define("y",      decodeCellID("y"),      {"cellID"})
      ;

  // map `(module,x,y)` to pixel hitmap bins
  auto dfHitmap = dfDecoded
      .Define("hitmapX", imod2hitmapX, {"module"})
      .Define("hitmapY", imod2hitmapY, {"module"})
      .Define("pixelX", pixelCoord, {"hitmapX","x"})
      .Define("pixelY", pixelCoord, {"hitmapY","y"})
      ;

  // count how many hits are inside the expected segmentation box
  auto countInBox = [&segXmin,&segXmax] (RVecL xVec, RVecL yVec) {
    return xVec[ xVec>=segXmin && xVec<=segXmax && yVec>=segXmin && yVec<=segXmax ].size();
  };
  auto countOutBox = [&segXmin,&segXmax] (RVecL xVec, RVecL yVec) {
    return xVec[ xVec<segXmin  || xVec>segXmax  || yVec<segXmin  || yVec>segXmax  ].size();
  };
  auto numInBox  = dfDecoded.Define( "numInBox",  countInBox,  {"x","y"} ).Sum("numInBox").GetValue();
  auto numOutBox = dfDecoded.Define( "numOutBox", countOutBox, {"x","y"} ).Sum("numOutBox").GetValue();
  fmt::print("{:=<60}\nNUMBER OF HITS OUTSIDE EXPECTED BOX: {} / {} ({:.4f}%)\n{:=<60}\n",
      "", numOutBox, numInBox+numOutBox, 100*Double_t(numOutBox)/Double_t(numInBox+numOutBox), "");

  // histograms
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // cellID field histograms
  auto fieldHists = std::vector({
    dfHitmap.Histo1D("system"),
    dfHitmap.Histo1D("sector"),
    dfHitmap.Histo1D("module"),
    dfHitmap.Histo1D("x"),
    dfHitmap.Histo1D("y")
  });
  const int segXmaxPlot = 10;
  auto segXY = dfHitmap.Histo2D(
      { "segXY", "CartesianGridXY;x;y",
        2*segXmaxPlot, -segXmaxPlot, segXmaxPlot,
        2*segXmaxPlot, -segXmaxPlot, segXmaxPlot },
      "x","y"
      );


  // pixel hitmap
  Double_t pixelXmin = dilation * wr.plotXmin;
  Double_t pixelXmax = dilation * wr.plotXmax;
  Double_t pixelYmin = dilation * wr.plotYmin;
  Double_t pixelYmax = dilation * wr.plotYmax;
  auto pixelHitmapModel = RDF::TH3DModel(
      "pixelHitmap", "Pixel Hit Map;x;y;sector",
      (Int_t)(pixelXmax-pixelXmin), pixelXmin, pixelXmax,
      (Int_t)(pixelYmax-pixelYmin), pixelYmin, pixelYmax,
      nSectors,0,double(nSectors)
      );
  auto pixelHitmap = fileType=="r" ? // weight by ADC counts, if reading digitized hits
    dfHitmap.Histo3D(pixelHitmapModel, "pixelX", "pixelY", "sector", inputCollection+".integral") :
    dfHitmap.Histo3D(pixelHitmapModel, "pixelX", "pixelY", "sector");

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
  auto expectedBox = new TBox(segXmin, segXmin, segXmax+1, segXmax+1);
  expectedBox->SetFillStyle(0);
  expectedBox->SetLineColor(kBlack);
  expectedBox->SetLineWidth(4);
  expectedBox->Draw("same");

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

    TString pixelHitmapTitle;
    if(fileType=="s")      pixelHitmapTitle = "Photon hits";
    else if(fileType=="r") pixelHitmapTitle = "ADC Counts";
    pixelHitmapTitle += Form(", sector %d",sec);
    if(evnumArg<0) pixelHitmapTitle += ", all events";
    else           pixelHitmapTitle += Form(", event %d",evnumArg);

    pixelHitmapSec[sec]->SetTitle(pixelHitmapTitle);
    pixelHitmapSec[sec]->Draw("colz");
    for(auto box : boxList) {
      box->Draw("same");
    };
    pixelHitmapSec[sec]->Draw("colz same");
    // pixelHitmapSec[sec]->GetXaxis()->SetRangeUser(475, 725);  // zoom in
    // pixelHitmapSec[sec]->GetYaxis()->SetRangeUser(-150, 150);
  };

  fmt::print("\n\npress ^C to exit.\n\n");
  mainApp.Run();
  return 0;
};
