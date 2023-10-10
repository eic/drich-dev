// test if the pixel gap cuts are working

#include <spdlog/spdlog.h>

#include <TApplication.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TStyle.h>

#include <podio/ROOTFrameReader.h>
#include <podio/Frame.h>
#include "DDRec/CellIDPositionConverter.h"

#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>

#include <services/geometry/richgeo/ReadoutGeo.h>
#include <services/geometry/richgeo/IrtGeoDRICH.h>

int main(int argc, char** argv) {

  // args
  if(argc<=1) {
    fmt::print("USAGE: {} [n/i] [file(default=out/rec.edm4hep.root)]\n", argv[0]);
    fmt::print(" [n/i]: n=non-interactive, i=interactive\n");
    return 2;
  }
  std::string interactiveOpt = std::string(argv[1]);
  std::string root_file_name = argc>2 ? std::string(argv[2]) : "out/rec.edm4hep.root";

  // set interactive mode
  bool interactiveOn = interactiveOpt=="i";
  TApplication *app;
  if(interactiveOn)
    app = new TApplication("app", &argc, argv);

  // logger
  auto logger = spdlog::default_logger()->clone("richgeo");
  logger->set_level(spdlog::level::trace);

  // geometry from compact file
  std::string DETECTOR_PATH(getenv("DETECTOR_PATH"));
  std::string DETECTOR_CONFIG(getenv("DETECTOR_CONFIG"));
  if(DETECTOR_PATH.empty() || DETECTOR_CONFIG.empty()) {
    logger->error("cannot find default compact file, since env vars DETECTOR_PATH and DETECTOR_CONFIG are not set");
    return 1;
  }
  auto compactFile = DETECTOR_PATH + "/" + DETECTOR_CONFIG + ".xml";
  dd4hep::Detector & det = dd4hep::Detector::getInstance();
  det.fromXML(compactFile);
  dd4hep::rec::CellIDPositionConverter cellid_converter(det);

  // ReadoutGeo
  richgeo::ReadoutGeo geo("DRICH", &det, &cellid_converter, logger);
  richgeo::IrtGeoDRICH drichGeo(&det, &cellid_converter, logger);
  // open input file
  auto reader = podio::ROOTFrameReader();
  reader.openFile(root_file_name);
  const std::string tree_name = "events";

  // local hits histogram
  double pi = TMath::Pi();  
  auto h = new TH2D("h","local MC SiPM hits",10000,-15,15,10000,-15,15);
  auto h1 = new TH1D("h1","Photon Incidence angle; rad",1000,-pi,pi);
  // event loop
  for(unsigned e=0; e<reader.getEntries(tree_name); e++) {
    logger->trace("EVENT {}", e);
    auto frame = podio::Frame(reader.readNextEntry(tree_name));
    const auto& hit_assocs  = frame.get<edm4eic::MCRecoTrackerHitAssociationCollection>("DRICHRawHitsAssociations");
    if(!hit_assocs.isValid())
      throw std::runtime_error("cannot find hit associations");

    for(const auto& hit_assoc : hit_assocs) {
      for(const auto& sim_hit : hit_assoc.getSimHits()) {

        auto cellID = sim_hit.getCellID();
        auto pos    = sim_hit.getPosition();
        auto mom    = sim_hit.getMomentum();
        TVector3 p; p.SetX(mom.x); p.SetY(mom.y); p.SetZ(mom.z);     
        auto normZ  = drichGeo.GetSensorSurface(cellID);
    
        double angle = normZ.Dot(p.Unit());
        h1->Fill(angle);
        dd4hep::Position pos_global(pos.x*dd4hep::mm, pos.y*dd4hep::mm, pos.z*dd4hep::mm);
        auto pos_local = geo.GetSensorLocalPosition(cellID, pos_global);
        h->Fill(pos_local.y()/dd4hep::mm, pos_local.x()/dd4hep::mm);

        if(logger->level() <= spdlog::level::trace) {
          logger->trace("pixel hit on cellID={:#018x}",cellID);
          auto print_pos = [&] (std::string name, dd4hep::Position p) {
            logger->trace("  {:>30} x={:.2f} y={:.2f} z={:.2f} [mm]: ", name, p.x()/dd4hep::mm,  p.y()/dd4hep::mm,  p.z()/dd4hep::mm);
          };
          print_pos("pos_local",  pos_local);
          print_pos("pos_global", pos_global);
          // auto dim = m_cellid_converter->cellDimensions(cellID);
          // for (std::size_t j = 0; j < std::size(dim); ++j)
          //   logger->trace("   - dimension {:<5} size: {:.2}",  j, dim[j]);
        }
      }
    }
  }

  gStyle->SetOptStat(0);
  auto c = new TCanvas(); c->Divide(2,1);
  c->cd(1);h->Draw();
  c->cd(2);h1->Draw();
  fmt::print("NUMBER OF DIGITIZED PHOTONS: {}\n", h->GetEntries());
  if(interactiveOn) {
    fmt::print("\n\npress ^C to exit.\n\n");
    app->Run();
  } else {
    c->SaveAs("out/pixel_gaps.png");
  }
}
