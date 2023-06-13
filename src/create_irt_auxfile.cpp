#include <cstdlib>
#include <iostream>
#include <spdlog/spdlog.h>

// ROOT
#include "TSystem.h"
#include "TFile.h"

// local
#include "WhichRICH.h"
#include <IrtGeoDRICH.h>
#include <IrtGeoPFRICH.h>

int main(int argc, char** argv) {

  // logger
  auto logger = spdlog::default_logger()->clone("richgeo");
  logger->set_level(spdlog::level::trace);

  // arguments
  std::string compactFileName = "";
  std::string irtAuxFileName  = "";
  if(argc<=1) {
    logger->info("USAGE: {} [d/p] [aux_file_name(optional)] [compact_file_name(optional)]",argv[0]);
    logger->info("    [d/p]: d for dRICH");
    logger->info("           p for pfRICH");
    return 2;
  }
  std::string zDirectionStr  = argv[1];
  if(argc>2) irtAuxFileName  = std::string(argv[2]);
  if(argc>3) compactFileName = std::string(argv[3]);

  // RICH-specific settings (also checks `zDirectionStr`)
  WhichRICH wr(zDirectionStr);
  if(!wr.valid) return 1;
  if(zDirectionStr=="p") {
    logger->info("NOTE: pfRICH is only in brycecanyon; updating $DETECTOR_CONFIG");
    setenv("DETECTOR_CONFIG","epic_brycecanyon",1);
  }

  // get compact file
  if(compactFileName=="") {
    std::string DETECTOR_PATH(getenv("DETECTOR_PATH"));
    std::string DETECTOR_CONFIG(getenv("DETECTOR_CONFIG"));
    if(DETECTOR_PATH.empty() || DETECTOR_CONFIG.empty()) {
      logger->error("cannot find default compact file, since env vars DETECTOR_PATH and DETECTOR_CONFIG are not set");
      return 1;
    }
    compactFileName = DETECTOR_PATH + "/" + DETECTOR_CONFIG + ".xml";
  }

  // start auxfile
  if(irtAuxFileName=="") irtAuxFileName = "geo/irt-"+wr.xrich+".root";
  auto irtAuxFile = new TFile(irtAuxFileName.c_str(),"RECREATE");

  // given DD4hep geometry from `compactFileName`, produce IRT geometry
  richgeo::IrtGeo *IG;
  if(zDirectionStr=="d")      IG = new richgeo::IrtGeoDRICH(compactFileName, logger);
  else if(zDirectionStr=="p") IG = new richgeo::IrtGeoPFRICH(compactFileName, logger);
  else return 1;

  // write IRT auxiliary file
  IG->GetIrtDetectorCollection()->Write();
  irtAuxFile->Close();
  logger->info("Wrote IRT Aux File: {}",irtAuxFileName);
}
