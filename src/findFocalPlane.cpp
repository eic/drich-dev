// After using debug optics mode 4/simulate.py test 14, calculate
// points of closest approach of reflected parallel photon beams,
// to get approximate focal region. 
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
#include "TVector3.h"
#include "TMatrix.h"
#include "TMatrixD.h"
#include "TMatrixDSparse.h"
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
std::vector<TVector3> d, e, fp, dirout;
int nphotons = 0;
int nAcc = 0;
TVector3 focalPoint(std::vector<TVector3> dvec, std::vector<TVector3> endvec){
  TMatrixD a(3,3), b(3,1);
  TArrayI row(3),col(3);
  
  for(int i = 0; i < 3; i++) row[i] = col[i] = i;
  TArrayD idarr(3); idarr.Reset(1.);
  TMatrixDSparse identity(3,3);
  identity.SetMatrixArray(3,row.GetArray(),col.GetArray(),idarr.GetArray());

  for(int i = 0; i < dvec.size(); i++){
    double darr[] = {dvec[i].X(),dvec[i].Y(),dvec[i].Z()};
    double earr[] = {endvec[i].X(),endvec[i].Y(),endvec[i].Z()};

    auto matd = TMatrixD(3,1,darr);  
    auto mate = TMatrixD(3,1,earr);  
    auto matdT = TMatrixD(matd);
    matdT.T();
    
    a += (identity - matd*matdT);
    b += (identity - matd*matdT)*mate;

  }
  auto c = (a.Invert()*b);
  TVector3 x(TMatrixDRow(c,0)(0),TMatrixDRow(c,1)(0),TMatrixDRow(c,2)(0));
  return x;
};

TVector3 avgDir(std::vector<TVector3> dvec){
  double xavg = 0;
  double yavg = 0;
  double zavg = 0;
  double n = 0;
  for(int i = 0; i < dvec.size(); i++){
    xavg += dvec[i].X();
    yavg += dvec[i].Y();
    zavg += dvec[i].Z();
    n+=1.0;
  }
  
  TVector3 outdir;
  if(n > 0) outdir.SetXYZ(xavg/n, yavg/n, zavg/n);
  return outdir;
};

int main(int argc, char** argv) {
  // setup
  TString infileN="sim_rich_vis.root";
  if(argc>1) infileN = TString(argv[1]);

  RDataFrame dfIn("events",infileN.Data());
  TString outfileN = infileN;
  outfileN(TRegexp("\\.root$"))=".";
  TFile *outfile = new TFile(outfileN+"plots.root","RECREATE");
  gStyle->SetOptStat(0);

  /* lambdas
   * - most of these transform an `RVec<T1>` to an `RVec<T2>` using `VecOps::Map` or `VecOps::Filter`
   * - see NPdet/src/dd4pod/dd4hep.yaml for POD syntax
   */
  cout << 0 << endl;
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
  auto hitPosX = [](RVec<Vector3d> v){ return Map(v,[](auto p){ return p.x; }); };
  auto hitPosY = [](RVec<Vector3d> v){ return Map(v,[](auto p){ return p.y; }); };
  auto hitPosZ = [](RVec<Vector3d> v){ return Map(v,[](auto p){ return p.z; }); };

  auto findReflected = [](RVec<MCParticleData> photons){ // want to skip non-reflected photons
    return Map( photons,[](auto p){
      auto imom = p.momentum;
      auto fmom = p.momentumAtEndpoint;
      return !(imom==fmom);
    });
  };
  auto dirVec = [](RVec<MCParticleData> parts){
    return Map(parts,[](auto p){
      auto dMom = TVector3(p.momentumAtEndpoint.x,p.momentumAtEndpoint.y,p.momentumAtEndpoint.z);
      return dMom.Unit();
      //return dMom;
    });
  };
  auto endVec = [](RVec<MCParticleData> parts){
    return Map(parts,[](auto p){
      return TVector3(p.endpoint.x/10., p.endpoint.y/10., p.endpoint.z/10.);
    });
  };        
  
  // transformations
  auto df1 = dfIn
    .Define("reflectedPhotons",findReflected,{"MCParticles"})
    .Define("direcVec",dirVec,{"MCParticles"})
    .Define("endVec",endVec,{"MCParticles"})
    ;
  
  df1.Foreach(
	      [d,e,fp,dirout,nphotons,nAcc](RVec<bool> refl, RVec<TVector3> dir, RVec<TVector3> end){		
		if( refl[0]  ){  d.push_back(dir[0]); e.push_back(end[0]); nAcc++;}
		nphotons++;
		if(nphotons==50){
		  if(nAcc>0){
		    fp.push_back(focalPoint(d,e));
		    dirout.push_back(avgDir(d));
		  }
		  nAcc = 0;
		  nphotons = 0;
		  d.clear();
		  e.clear();		  
		}
	      },
	      {"reflectedPhotons","direcVec","endVec"}
	      );
  auto dfFinal = df1;
  cout << 1 << endl;
  cout << "dir len " << dirout.size() << endl;
  FILE * outtxt = fopen("focalPoints.txt","w");
  for(int i = 0 ; i < fp.size(); i++){
    if( std::abs(fp[i].X()) < 1000 && std::abs(fp[i].Y()) < 1000 && std::abs(fp[i].Z()) < 1000){      
      fprintf(outtxt, "%lf %lf %lf %lf %lf %lf \n", fp[i].X(), fp[i].Y(), fp[i].Z(), dirout[i].X(), dirout[i].Y(), dirout[i].Z());
    }
  }
  fclose(outtxt);

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

