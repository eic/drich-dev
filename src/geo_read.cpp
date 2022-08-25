#include <cstdlib>
#include <iostream>

// ROOT
#include "TSystem.h"
#include "TFile.h"

// DD4Hep
#include "DD4hep/Detector.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DD4hep/DD4hepUnits.h"

// IRT
#include "CherenkovDetectorCollection.h"
#include "CherenkovPhotonDetector.h"
#include "CherenkovRadiator.h"
#include "OpticalBoundary.h"
#include "ParametricSurface.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

int main(int argc, char** argv) {

  // compact file name
  string DETECTOR_PATH = string(getenv("DETECTOR_PATH"));
  string DETECTOR = string(getenv("DETECTOR"));
  // string DETECTOR = string(getenv("DETECTOR")) + "_drich_only";
  string compactFile = DETECTOR_PATH + "/" + DETECTOR + ".xml";

  // irt auxfile name
  string irtAuxFileName = string("geo/irt-drich.root");

  // arguments
  if(argc>1) compactFile    = string(argv[1]);
  if(argc>2) irtAuxFileName = string(argv[2]);
  cout << "compactFile = " << compactFile << endl;

  // full detector handle
  auto det = &(dd4hep::Detector::getInstance());
  det->fromXML(compactFile);

  // managers
  // auto volman  = det->volumeManager();
  // auto surfman = det->surfaceManager();
  // det->apply("DD4hepVolumeManager",0,0);

  // cellID decoder
  auto decoder = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*det);

  // dRICH handle
  const string richName = "DRICH";
  auto detRich = det->detector(richName);
  auto posRich = detRich.placement().position();

  // start auxfile
  auto irtAuxFile  = new TFile(irtAuxFileName.c_str(),"RECREATE");
  auto irtGeometry = new CherenkovDetectorCollection();
  auto irtDetector = irtGeometry->AddNewDetector(richName.c_str());

  // helpers
  auto id2secmod = [](int id){
    return std::pair<int,int>(id&0x7,id>>3); // FIXME: use `decoder`
  };
  auto starts_with = [](std::string str, const char *pat) {
    return str.find(string(pat)) != string::npos; // true if `str` starts with `pat`
  };
  auto loop_de = [&detRich,&starts_with](const char *pat, std::function<void(dd4hep::DetElement)> block) {
    for(auto const& [de_name, de] : detRich.children()) {
      if(starts_with(de_name,pat)) {
        cout << "found " << de_name << endl;
        block(de);
      }
    }
  };


  // begin envelope
  /* FIXME: have no connection to GEANT G4LogicalVolume pointers; however all is needed
   * is to make them unique so that std::map work internally; resort to using integers,
   * who cares; material pointer can seemingly be '0', and effective refractive index
   * for all radiators will be assigned at the end by hand; FIXME: should assign it on
   * per-photon basis, at birth, like standalone GEANT code does;
   */
  int nSectors = 6;
  TVector3 normX(1, 0, 0); // normal vectors
  TVector3 normY(0, -1, 0);

  auto vesselZmin = det->constant<double>("DRICH_zmin");
  string gasvolMatName;
  auto get_gasvolMatName = [&gasvolMatName] (dd4hep::DetElement gasvol) {
    gasvolMatName = gasvol.volume().material().name();
  };
  loop_de("gasvol",get_gasvolMatName);
  cout << gasvolMatName << endl;

  auto surfEntrance = new FlatSurface(
      (1 / dd4hep::mm) * TVector3(0, 0, vesselZmin),
      normX,
      normY
      );
  for (int sec = 0; sec < nSectors; sec++) {
    auto rad = irtGeometry->SetContainerVolume(
        irtDetector,             // Cherenkov detector
        "GasVolume",             // name
        sec,                     // path
        (G4LogicalVolume*)(0x0), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial (inaccessible?)
        surfEntrance             // surface
        );
    rad->SetAlternativeMaterialName(gasvolMatName.c_str());
  }


  //////////////////////////////////
  // begin legacy juggler geosvc code
  //

  /*
  

  // FIXME: Get access to the readout structure decoder
  // irtDetector->SetReadoutCellMask( ... )

  // set IRT sensors // FIXME: '0' stands for the unknown (and irrelevant) G4LogicalVolume;
  auto pd = new CherenkovPhotonDetector(0, 0);
  irtGeometry->AddPhotonDetector(irtDetector, 0, pd);
  
  // loop over RICH detector elements
  bool radClosed = false;
  for(auto const& [richDEname, richDE] : detRich.children()) {

    // get element attributes
    //if(debug_geosvc) info() << "FOUND RICH element: " << richDEname << endmsg;
    auto dePos = posRich + richDE.placement().position();
    int deID = richDE.id();

    // aerogel and filter ~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if(richDEname.find("aerogel")!=std::string::npos || richDEname.find("filter")!=std::string::npos) {
      double thickness = 2 * richDE.volume().boundingBox().dimensions()[2];
      int isec = deID;
      if(debug_geosvc) {
        info() << "RICH RADIATOR: " << richDEname
               << "\n\t(x,y,z)-position = " << dePos.x() << ", " << dePos.y() << ", " << dePos.z()
               << "\n\tsector = " << isec
               << "\n\tthickness = " << thickness
               << endmsg;
      }
      if(isec<nSectors) { // FIXME: need sector loop?
        if(richDEname.find("aerogel")!=std::string::npos) {
          auto aerogelSurf = new FlatSurface( (1/mm)*TVector3(0,0,dePos.z()), normX, normY);
          irtGeometry->AddFlatRadiator(irtDetector, "Aerogel", isec, (G4LogicalVolume*)(0x1), 0, aerogelSurf, thickness/mm);
        } else { // elif filter
          auto filterSurf = new FlatSurface( (1/mm)*TVector3(0,0,dePos.z()), normX, normY); // NOTE: there is an airgap in geometry
          irtGeometry->AddFlatRadiator(irtDetector, "Filter", isec, (G4LogicalVolume*)(0x2), 0, filterSurf, thickness/mm);
        }
      }
    }

    // mirrors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if(richDEname.find("mirror")!=std::string::npos) {
      int isec = deID;

      // spherical mirror DetElement solid is a BooleanSolid; to get the sphere attributes
      // we need to access both the primitive Sphere and its relative positioning
      dd4hep::Solid sphPrim;
      auto sphPos = dePos;
      findPrimitive("TGeoSphere",richDE.solid(),sphPrim,sphPos); // get sphere primitive
      auto sph = (dd4hep::Sphere) sphPrim;

      // for some reason, the sector z-rotation is not accounted for in `findPrimitive`, so we correct for it here:
      if(richDE.placement().matrix().IsRotAboutZ()) {
        Double_t sphPosArrLocal[3], sphPosArrMaster[3];
        sphPos.GetCoordinates(sphPosArrLocal);
        richDE.placement().matrix().LocalToMaster(sphPosArrLocal,sphPosArrMaster);
        sphPos.SetCoordinates(sphPosArrMaster);
      } else error() << "richDE.placement().matrix() is not a z-rotation; cross check mirror center coords!!!" << endmsg;

      // mirror attributes
      double mirrorRadius = sph.rMin();
      if(debug_geosvc) {
        info() << "RICH MIRROR: " << richDEname
               << "\n\t(x,y,z)-position = " << sphPos.x() << ", " << sphPos.y() << ", " << sphPos.z()
               << "\n\tsector = " << isec
               << "\n\tradius = " << mirrorRadius
               << endmsg;
      }
      auto mirrorSurf = new SphericalSurface(
          (1/mm)*TVector3(sphPos.x(),sphPos.y(),sphPos.z()),
          mirrorRadius/mm
          );
      irtDetector->AddOpticalBoundary(isec, new OpticalBoundary(irtDetector->GetContainerVolume(), mirrorSurf, false));
    }

    // sensors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if(richDEname.find("sensor")!=std::string::npos) {
      int ielem = deID;
      if(debug_geosvc) {
        auto secmod = id2secmod(ielem);
        int isec = secmod.first;
        int imod = secmod.second;
        info() << "RICH SENSOR: " << richDEname
               << "\n\t(x,y,z)-position = " << dePos.x() << ", " << dePos.y() << ", " << dePos.z()
               << "\n\tid   = " << ielem
               << "\n\tisec = " << isec
               << "\n\timod = " << imod
               << endmsg;
      }

      // FIXME: why not `dePos.x(), dePos.y(), dePos.z()`? why the orientation `normX.cross(normY)`, rather than along sphere radius?
      auto sensorSurf = new FlatSurface( (1/mm)*TVector3(0.0, 0.0, dePos.z()), normX, normY);
      irtDetector->CreatePhotonDetectorInstance(0, pd, ielem, sensorSurf);

      // close IRT gas radiator by hand (once) // FIXME: why don't we do this after this DetElement loop?
      if(!radClosed && zDirection==-1) { // FIXME: needs to be done for dRICH too
        irtDetector->GetRadiator("GasVolume")->m_Borders[0].second = dynamic_cast<ParametricSurface*>(sensorSurf);
        radClosed = true;
      }
    }

    */
  irtAuxFile->Close();
}
