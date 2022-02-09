// test readout segmentation
#include <cstdlib>
#include <iostream>
#include <bitset>
#include <map>
#include <vector>

// ROOT
#include "TSystem.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TBox.h"
#include "ROOT/RDataFrame.hxx"
//#include "ROOT/RDFHelpers.hxx" // for RDF::RunGraphs

using std::cout;
using std::cerr;
using std::endl;

using namespace ROOT;
using lvec = VecOps::RVec<Long64_t>;

int main(int argc, char** argv) {

  // arguments
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  TString infileN="out/sim_run.root";
  if(argc>1) infileN = TString(argv[1]);

  // settings
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // number of pixels along sensor side
  const Int_t numPx = 8;

  // dilations: for re-scaling module positions and segment positions
  // for drawing; if you change `numPx`, consider tuning these parameters
  // as well
  Long64_t dilation = 4;


  // setup
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // define application environment, to keep canvases open
  TApplication mainApp("mainApp",&argc,argv);

  // enable multi-threading
  EnableImplicitMT();

  // read tree into dataframe
  RDataFrame dfIn("events",infileN.Data());

  // load sensor position LUT
  std::map<Long64_t,std::pair<Long64_t,Long64_t>> modCoordMap;
  TTree *modCoordTr = new TTree();
  modCoordTr->ReadFile("text/sensorLUT.dat","module/L:x/F:y/F");
  Long64_t moduleSens,xSens,ySens; Float_t xSensF,ySensF;
  modCoordTr->SetBranchAddress("module",&moduleSens);
  modCoordTr->SetBranchAddress("x",&xSensF);
  modCoordTr->SetBranchAddress("y",&ySensF);
  std::vector<TBox*> boxList;
  for(Long64_t e=0; e<modCoordTr->GetEntries(); e++) {
    modCoordTr->GetEntry(e);
    xSens = (Long64_t)(dilation*xSensF+0.5);
    ySens = (Long64_t)(dilation*ySensF+0.5);
    modCoordMap.insert(std::pair<Long64_t,std::pair<Long64_t,Long64_t>>(
          moduleSens, std::pair<Long64_t,Long64_t>(xSens,ySens)
          ));
    boxList.push_back(new TBox(
          xSens,
          ySens,
          xSens + numPx,
          ySens + numPx
          ));
  };
  

  // lambdas
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // decode cellID vector, given offset and length
  // TODO: replace with dd4hep::rec::CellIDPositionConverter 
  // (see https://eicweb.phy.anl.gov/EIC/tutorials/ip6_tutorial_1/-/blob/master/scripts/tutorial1_hit_position.cxx)
  auto decodeID = [] (lvec ids, int offset, int length) {
    Long64_t div = (Long64_t) pow(2,offset);
    Long64_t mask = (Long64_t) pow(2,length) - 1;
    auto result = (ids/div) & mask;
    return result;
  };
  // decoder for signed indicators (conversion may not be correct)
  /*auto decodeIDsigned = [] (lvec ids, int offset, int length) {
    Long64_t div = (Long64_t) pow(2,offset);
    Long64_t mask = (Long64_t) pow(2,length) - 1;
    lvec result;
    for(auto i : ids) {
      auto r = (i >> offset) & mask;
      if(r>=pow(2,length-2)) r -= (Long64_t) pow(2,length-1);
      result.emplace_back(r);
    };
    // debugging prints
    for(auto b : ids) cout << std::bitset<64>(b) << endl;
    cout << "(" << offset << "," << length << ") " << result << endl;
    for(auto b : result) cout << std::bitset<16>(b) << endl;
    return result;
  };*/
  // decoders: indicator -> cellID offset and length
  auto detDecode = [&decodeID] (lvec ids) { return decodeID(ids,0,8); };
  auto modDecode = [&decodeID] (lvec ids) { return decodeID(ids,8,12); };
  auto xDecode   = [&decodeID] (lvec ids) { return decodeID(ids,20,16); };
  auto yDecode   = [&decodeID] (lvec ids) { return decodeID(ids,36,16); };


  // convert module number to module histogram position
  auto modCoordLU = [&modCoordMap] (lvec mods, int c) {
    lvec result;
    Double_t pos;
    for(auto m : mods) {
      try { pos = (c==0) ? modCoordMap[m].first : modCoordMap[m].second; } 
      catch (const std::out_of_range &ex) {
        cerr << "ERROR: cannot find module " << m << endl;
        pos = 0;
      };
      result.emplace_back(pos);
    };
    return result;
  };
  auto modCoordX = [&modCoordLU] (lvec ids) { return modCoordLU(ids,0); };
  auto modCoordY = [&modCoordLU] (lvec ids) { return modCoordLU(ids,1); };

  // convert (module,segment) to pixel histogram position
  auto pixelCoord = [] (lvec modXs, lvec segXs) {
    return modXs + segXs;
  };


  // transformations
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // decode cellID
  auto dfDecoded = dfIn
      .Alias("id","ERICHHits.cellID")
      .Define("det", detDecode, {"id"})
      .Define("mod", modDecode, {"id"})
      .Define("segX", xDecode, {"id"})
      .Define("segY", yDecode, {"id"})
      ;

  // convert modules to histogram positions
  auto dfModded = dfDecoded
      .Define("modX", modCoordX, {"mod"})
      .Define("modY", modCoordY, {"mod"})
      ;

  // convert (module,segment) positions to pixel histogram position
  auto dfPixels = dfModded
      .Define("pixelX", pixelCoord, {"modX","segX"})
      .Define("pixelY", pixelCoord, {"modY","segY"})
      ;

  // final dataframe
  auto dfFinal = dfPixels;


  // actions
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // book histograms
  auto detHist = dfFinal.Histo1D("det");
  auto modHist = dfFinal.Histo1D("mod");
  auto xHist = dfFinal.Histo1D("segX");
  auto yHist = dfFinal.Histo1D("segY");

  // pixel hits
  Double_t pixelXmin = dilation * -100;
  Double_t pixelXmax = dilation * 100;
  Double_t pixelYmin = dilation * -100;
  Double_t pixelYmax = dilation * 100;
  auto pixelHits = dfFinal.Histo2D(
      { "pixelHits","pixel hits;x;y",
        (Int_t)(pixelXmax-pixelXmin), pixelXmin, pixelXmax,
        (Int_t)(pixelYmax-pixelYmin), pixelYmin, pixelYmax },
      "pixelX","pixelY"
      );

  // execute concurrently
  //RDF::RunGraphs({ detHist, modHist, xHist, yHist, pixelHits });


  // execution
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // draw segmentation indicators
  TCanvas * c = new TCanvas();
  c->Divide(2,2);
  for(int pad=1; pad<=4; pad++) c->GetPad(pad)->SetLogy();
  c->cd(1); detHist->Draw();
  c->cd(2); modHist->Draw();
  c->cd(3); xHist->Draw();
  c->cd(4); yHist->Draw();


  // draw pixel hits
  /*
  Bool_t singleCanvas = false;
  if(singleCanvas) { c = new TCanvas(); c->Divide(3,2); };
  Int_t secBin;
  const Int_t nSec = 6;
  TH2D *pixelHitsSec[nSec];
  for(int sec=0; sec<6; sec++) {
    if(singleCanvas) c->cd(sec+1); else c = new TCanvas();
    secBin = pixelHits->GetZaxis()->FindBin((Float_t)sec);
    pixelHits->GetZaxis()->SetRange(secBin,secBin);
    pixelHitsSec[sec] = (TH2D*) pixelHits->Project3D("yx");
    pixelHitsSec[sec]->SetName(Form("pixelHits_s%d",sec));
    pixelHitsSec[sec]->SetTitle(Form("pixel hits sector %d",sec));
    pixelHitsSec[sec]->Draw("colz");
    for(auto box : boxList) {
      box->SetFillStyle(0);
      box->Draw("same");
    };
    pixelHitsSec[sec]->Draw("colz same");
  };
  */

  c = new TCanvas();
  pixelHits->Draw("colz");
  for(auto box : boxList) {
    box->SetFillStyle(0);
    box->Draw("same");
  };
  pixelHits->Draw("colz same");


  cout << "\n\npress ^C to exit.\n\n";
  mainApp.Run();
  return 0;
};
