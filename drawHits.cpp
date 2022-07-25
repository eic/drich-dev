// draw hits, and make some other related plots
// (cf. drawSegmentation.cpp for readout)
#include <cstdlib>
#include <iostream>

// ROOT
#include "TSystem.h"
#include "TStyle.h"
#include "TRegexp.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TBox.h"
#include "ROOT/RDataFrame.hxx"

// edm4hep
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"

using std::cout;
using std::cerr;
using std::endl;

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace edm4hep;

TCanvas *CreateCanvas(TString name, Bool_t logx=0, Bool_t logy=0, Bool_t logz=0);

int main(int argc, char** argv) {

  // setup
  TString infileN;
  if(argc<=1) {
    cout << "USAGE: " << argv[0] << " [simulation root file]" << endl;
    return 2;
  }
  infileN = TString(argv[1]);
  //TApplication mainApp("mainApp",&argc,argv); // keep canvases open
  //EnableImplicitMT();
  RDataFrame dfIn("events",infileN.Data());
  TString outfileN = infileN;
  outfileN(TRegexp("\\.root$"))=".";
  TFile *outfile = new TFile(outfileN+"plots.root","RECREATE");
  gStyle->SetOptStat(0);


  /* lambdas
   * - most of these transform an `RVec<T1>` to an `RVec<T2>` using `VecOps::Map` or `VecOps::Filter`
   * - see NPdet/src/dd4pod/dd4hep.yaml for POD syntax
   */
  // calculate number of hits
  auto numHits = [](RVec<SimTrackerHitData> hits) { return hits.size(); };
  // calculate momentum magnitude for each particle (units=GeV)
  // TODO: edm4hep::Vector3f really has no magnitude function!?
  auto momentum = [](RVec<MCParticleData> parts){
    return Map(parts,[](auto p){
        auto mom = p.momentum;
        return sqrt( mom[0]*mom[0] + mom[1]*mom[1] + mom[2]*mom[2] );
        });
  };
  // filter for thrown particles
  auto isThrown = [](RVec<MCParticleData> parts){
    return Filter(parts,[](auto p){
        return p.generatorStatus==1;
        });
  };
  // get positions for each hit (units=cm)
  auto hitPos = [](RVec<SimTrackerHitData> hits){ return Map(hits,[](auto h){ return h.position; }); };
  auto hitPosX = [](RVec<Vector3d> v){ return Map(v,[](auto p){ return p.x/10; }); };
  auto hitPosY = [](RVec<Vector3d> v){ return Map(v,[](auto p){ return p.y/10; }); };
  auto hitPosZ = [](RVec<Vector3d> v){ return Map(v,[](auto p){ return p.z/10; }); };


  // transformations
  auto df1 = dfIn
    .Define("thrownParticles",isThrown,{"MCParticles"})
    .Define("thrownP",momentum,{"thrownParticles"})
    .Define("numHits",numHits,{"DRICHHits"})
    .Define("hitPos",hitPos,{"DRICHHits"})
    .Define("hitX",hitPosX,{"hitPos"})
    .Define("hitY",hitPosY,{"hitPos"})
    ;
  auto dfFinal = df1;


  // actions
  auto hitPositionHist = dfFinal.Histo2D(
      { "hitPositions","dRICh hit positions (units=cm)",
      1000,-200,200, 1000,-200,200 },
      "hitX","hitY"
      );
  auto numHitsVsThrownP = dfFinal.Histo2D(
      { "numHitsVsThrownP","number of dRICh hits vs. thrown momentum", 
      65,0,65, 100,0,200 },
      "thrownP","numHits"
      ); // TODO: cut opticalphotons (may not be needed, double check PID)


  // execution
  TCanvas *canv;
  canv = CreateCanvas("hits",0,0,1);
  hitPositionHist->Draw("colz");
  hitPositionHist->GetXaxis()->SetRangeUser(100,200);
  hitPositionHist->GetYaxis()->SetRangeUser(-60,60);
  canv->Print(outfileN+"hits.png");
  canv->Write();
  //
  canv = CreateCanvas("photon_yield");
  numHitsVsThrownP->Draw("box");
  TProfile * aveHitsVsP;
  aveHitsVsP = numHitsVsThrownP->ProfileX("_pfx",1,-1,"i"); // TODO: maybe not the right errors, see TProfile::BuildOptions `i`
  aveHitsVsP->SetLineColor(kBlack);
  aveHitsVsP->SetLineWidth(3);
  aveHitsVsP->Draw("same");
  canv->Print(outfileN+"photon_count.png");
  canv->Write();
  aveHitsVsP->Write("aveHitsVsP");
  outfile->Close();


  // exit
  //cout << "\n\npress ^C to exit.\n\n";
  //mainApp.Run(); // keep canvases open
  return 0;
};


TCanvas *CreateCanvas(TString name, Bool_t logx, Bool_t logy, Bool_t logz) {
  TCanvas *c = new TCanvas("canv_"+name,"canv_"+name,800,600);
  c->SetGrid(1,1);
  if(logx) c->SetLogx(1);
  if(logy) c->SetLogy(1);
  if(logz) c->SetLogz(1);
  return c;
};

