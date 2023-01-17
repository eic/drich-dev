// dRICH event display


//////////////////////////////////
// if defined, keep TCanvas open for interactive usage (and with extra histograms)
//#define INTERACTIVE_USE
//////////////////////////////////


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
    fmt::print("\nUSAGE: {} [d/p] [s/r] [input_root_file] [event_num_min] [event_num_max]\n\n",argv[0]);
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
    fmt::print("    [event_num_min]: minimum event number (optional)\n");
    fmt::print("         - if unspecified, draw sum of all events\n");
    fmt::print("         - if specified, but without [event_num_max], draw only\n");
    fmt::print("           this event\n");
    fmt::print("         - if specified with [event_num_max], draw the range\n");
    fmt::print("           of events, ONE at a time\n");
    fmt::print("    [event_num_max]: maximum event number (optional)\n");
    fmt::print("         - set to 0 if you want the maximum possible\n");
    fmt::print("\n");
    fmt::print("\n");
    fmt::print("NOTE: INTERACTIVE_USE mode is ");
#ifdef INTERACTIVE_USE
    fmt::print("ON: TCanvases (and extra histograms) will remain open\n");
#else
    fmt::print("OFF: TCanvases will saved as PNG files\n");
    fmt::print("- FIXME: there is still a slow memory leak, don't run too many\n");
#endif
    fmt::print("- this setting is hard-coded in {}.cpp\n",argv[0]);
    return 2;
  }
  std::string zDirectionStr = argv[1];
  std::string fileType      = argv[2];
  TString     infileN       = TString(argv[3]);
  int         evnumMin      = argc>4 ? std::atoi(argv[4]) : -1;
  int         evnumMax      = argc>5 ? std::atoi(argv[5]) : -1;
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
  Bool_t singleCanvas = true; // if true, draw all hitmaps on one canvas
#ifndef INTERACTIVE_USE
  singleCanvas = true;
#endif

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

  // setup
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef INTERACTIVE_USE
  // define application environment, to keep canvases open
  TApplication mainApp("mainApp",&argc,argv);
#endif

  // main dataframe
  RDataFrame dfIn("events",infileN.Data());
  auto numEvents = dfIn.Count().GetValue();

  // determine event numbers to read
  std::vector<std::pair<int,int>> evnumRanges;
  if(evnumMin<0) {
    fmt::print("Reading all events\n");
    evnumRanges.push_back({ 0, 0 });
  }
  else if(evnumMax<0) {
    fmt::print("Reading only event number {}\n",evnumMin);
    evnumRanges.push_back({ evnumMin, evnumMin+1 });
  }
  else {
    if(evnumMax==0)
      evnumMax = numEvents-1;
    if(evnumMax>=numEvents) {
      fmt::print("WARNING: there are only {} events\n",numEvents);
      evnumMax = numEvents-1;
    }
    fmt::print("Reading event numbers {} to {}, one at a time\n",evnumMin,evnumMax);
    for(int e=evnumMin; e<=evnumMax; e++)
      evnumRanges.push_back({ e, e+1 });
#ifdef INTERACTIVE_USE
    fmt::print(stderr,"ERROR: cannot yet run with INTERACTIVE_USE on an event number range; change it in {}.cpp and rebuild\n",argv[0]);
    return 1;
#endif
  }


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

  // loop over event number(s)
  for(auto& [evnum,evnum_stop] : evnumRanges) {

    // cut on specified event(s)
    auto dfFinal = dfHitmap.Range(evnum,evnum_stop);

    // histograms
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // cellID field histograms
    auto fieldHists = std::vector({
      dfFinal.Histo1D("system"),
      dfFinal.Histo1D("sector"),
      dfFinal.Histo1D("module"),
      dfFinal.Histo1D("x"),
      dfFinal.Histo1D("y")
    });
    const int segXmaxPlot = 10;
    auto segXY = dfFinal.Histo2D(
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
      dfFinal.Histo3D(pixelHitmapModel, "pixelX", "pixelY", "sector", inputCollection+".integral") :
      dfFinal.Histo3D(pixelHitmapModel, "pixelX", "pixelY", "sector");

    // draw
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    TCanvas *c;
#ifdef INTERACTIVE_USE
    // draw cellID field histograms
    c = new TCanvas();
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
#endif

    // draw pixel hitmap
    if(singleCanvas) { c = new TCanvas("canv","canv",3*700,2*600); c->Divide(3,2); };
    int secBin;
    TH2D *pixelHitmapSec[nSectors];
    for(int sec=0; sec<nSectors; sec++) {
      if(singleCanvas) c->cd(sec+1); else c = new TCanvas();
      secBin = pixelHitmap->GetZaxis()->FindBin(Double_t(sec));
      pixelHitmap->GetZaxis()->SetRange(secBin,secBin);

      pixelHitmapSec[sec] = (TH2D*) pixelHitmap->Project3D("yx");
      if(fileType=="r") pixelHitmapSec[sec]->SetMaximum(4096);
      pixelHitmapSec[sec]->SetName(Form("pixelHitmap_s%d",sec));

      TString pixelHitmapTitle;
      if(fileType=="s")      pixelHitmapTitle = "Photon hits";
      else if(fileType=="r") pixelHitmapTitle = "Digitized hits";
      pixelHitmapTitle += Form(", sector %d",sec);
      if(evnumMin<0) pixelHitmapTitle += ", all events";
      else           pixelHitmapTitle += Form(", event %d",evnum);

      pixelHitmapSec[sec]->SetTitle(pixelHitmapTitle);
      pixelHitmapSec[sec]->Draw("colz");
      for(auto box : boxList) {
        box->Draw("same");
      };
      pixelHitmapSec[sec]->Draw("colz same");
      // pixelHitmapSec[sec]->GetXaxis()->SetRangeUser(475, 725);  // zoom in
      // pixelHitmapSec[sec]->GetYaxis()->SetRangeUser(-150, 150);
    };

    // either hold the TCanvases open, or save them as PNG files
#ifdef INTERACTIVE_USE
    fmt::print("\n\npress ^C to exit.\n\n");
    mainApp.Run();
#else
    gROOT->ProcessLine(".! mkdir -p out/ev");
    c->SaveAs(Form("out/ev/%s.png",fmt::format("{:08}",evnum).c_str()));
    // cleanup and avoid memory leaks # FIXME: refactor this... and there is still a slow leak...
    delete c;
    for(int sec=0; sec<nSectors; sec++) delete pixelHitmapSec[sec];
#endif

  } // end evnumRanges loop

#ifndef INTERACTIVE_USE
  fmt::print("\n\nEvent display images written to out/ev/*.png\n\n");
#endif

  return 0;
};
