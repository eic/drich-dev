// test if the pixel gap cuts are working

#include <spdlog/spdlog.h>

#include <TApplication.h>
#include <TH2D.h>
#include <TCanvas.h>
#include "TSystem.h"
#include "TStyle.h"
#include "TRegexp.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TBox.h"
#include "ROOT/RDataFrame.hxx"


#include <podio/ROOTFrameReader.h>
#include <podio/Frame.h>
#include "DDRec/CellIDPositionConverter.h"

#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <services/geometry/richgeo/ReadoutGeo.h>
#include <services/geometry/richgeo/IrtGeoDRICH.h>

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace edm4hep;

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
  auto h1 = new TH1D("h1","Photon Incidence angle (filtered ring); degrees",180,0,90);
  auto h2 = new TH2D("h2","Photon Incidence angle Vs Lambda(all);#lambda(nm);angle(degrees)",2000,0.,1000.,180,0.,90.);
  auto h3 = new TH2D("h3","Photon Incidence angle Vs Lambda(X filtered);#lambda(nm);angle(degrees)",2000,0.,1000.,180,0.,90.);
  auto h4 = new TH2D("h4","sim hits; X;Y",4000,-2000,2000,4000,-2000,2000);
  // event loop
  for(unsigned e=0; e<reader.getEntries(tree_name); e++) {
    logger->trace("EVENT {}", e);
    auto frame = podio::Frame(reader.readNextEntry(tree_name));
    const auto& hit_assocs  = frame.get<edm4eic::MCRecoTrackerHitAssociationCollection>("DRICHRawHitsAssociations");
    auto isThrown = [](RVec<MCParticleData> parts){
      return Filter(parts, [](auto p){ return p.generatorStatus==1; } );
    };
    //if(!isThrown) continue;
    if(!hit_assocs.isValid())
      throw std::runtime_error("cannot find hit associations");

    for(const auto& hit_assoc : hit_assocs) {
      for(const auto& sim_hit : hit_assoc.getSimHits()) {
        auto cellID = sim_hit.getCellID();
        auto pos    = sim_hit.getPosition();
	h4->Fill(pos.x,pos.y);
        auto mom    = sim_hit.getMomentum();
        TVector3 p; p.SetX(mom.x); p.SetY(mom.y); p.SetZ(mom.z);
	double Lambda = (1239.8/(p.Mag()*1.0e+9));
        auto normZ  = drichGeo.GetSensorSurface(cellID);
	double cosAng = (normZ.Unit()).Dot(p.Unit());
	double angle = pi- acos(cosAng);
	h2->Fill(Lambda,angle*(180/pi));
	if (pos.x> 1180 && pos.x<1260){
	  if(pos.y> -45 && pos.y<50){
	    h1->Fill(angle*(180/pi));
	  }
	}
	if(pos.x>1500 || pos.x<1000)
	  h3->Fill(Lambda,angle*(180/pi));
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
  auto c = new TCanvas(); c->Divide(2,2);
  c->cd(1);h4->Draw();
  c->cd(2);gPad->SetLogy();h1->Draw();
  c->cd(3);h2->Draw("colz");
  c->cd(4);h3->Draw("colz");
  fmt::print("NUMBER OF DIGITIZED PHOTONS: {}\n", h->GetEntries());
  if(interactiveOn) {
    fmt::print("\n\npress ^C to exit.\n\n");
    app->Run();
  } else {
    c->SaveAs("out/pixel_gaps.png");
    c->SaveAs("out/pixel_gaps.root");
  }
}
