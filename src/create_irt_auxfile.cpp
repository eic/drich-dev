#include <cstdlib>
#include <iostream>

// ROOT
#include "TSystem.h"
#include "TFile.h"

// local
#include "WhichRICH.h"
#include "IrtGeoDRICH.h"
#include "IrtGeoPFRICH.h"

int main(int argc, char** argv) {

  // arguments
  std::string compactFileName = "";
  std::string irtAuxFileName  = "";
  if(argc<=1) {
    fmt::print("\nUSAGE: {} [d/p] [compact_file_name(optional)] [aux_file_name(optional)]\n\n",argv[0]);
    fmt::print("    [d/p]: d for dRICH\n");
    fmt::print("           p for pfRICH\n");
    return 2;
  }
  std::string zDirectionStr  = argv[1];
  if(argc>2) compactFileName = std::string(argv[2]);
  if(argc>3) irtAuxFileName  = std::string(argv[3]);

  // RICH-specific settings (also checks `zDirectionStr`)
  WhichRICH wr(zDirectionStr);
  if(!wr.valid) return 1;
  if(zDirectionStr=="p") {
    fmt::print("NOTE: pfRICH is only in brycecanyon; updating $DETECTOR_CONFIG\n");
    setenv("DETECTOR_CONFIG","epic_brycecanyon",1);
  }

  // start auxfile
  if(irtAuxFileName=="") irtAuxFileName = "geo/irt-"+wr.xrich+".root";
  auto irtAuxFile = new TFile(irtAuxFileName.c_str(),"RECREATE");

  // given DD4hep geometry from `compactFileName`, produce IRT geometry
  richgeo::IrtGeo *IG;
  if(zDirectionStr=="d")      IG = new richgeo::IrtGeoDRICH(compactFileName, true);
  else if(zDirectionStr=="p") IG = new richgeo::IrtGeoPFRICH(compactFileName, true);
  else return 1;

  // write IRT auxiliary file
  IG->GetIrtDetectorCollection()->Write();
  irtAuxFile->Close();
  fmt::print("\nWrote IRT Aux File: {}\n\n",irtAuxFileName);
}
