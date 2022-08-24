#include <cstdlib>
#include <iostream>

// ROOT
#include "TSystem.h"
#include "TFile.h"

// DD4Hep
#include "DD4hep/Detector.h"
#include "DDRec/CellIDPositionConverter.h"

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
  string det_path = string(getenv("DETECTOR_PATH"));
  string det_name = string(getenv("DETECTOR")) + "_drich_only";
  string compact_file = det_path + "/" + det_name + ".xml";

  // irt auxfile name
  string irt_auxfile_name = string("geo/irt-drich.root");

  // arguments
  if(argc>1) compact_file     = string(argv[1]);
  if(argc>2) irt_auxfile_name = string(argv[2]);
  cout << "compact_file = " << compact_file << endl;

  // full detector handle
  auto det = &(dd4hep::Detector::getInstance());
  det->fromCompact(compact_file);

  // managers
  auto volman  = det->volumeManager();
  auto surfman = det->surfaceManager();
  // det->apply("DD4hepVolumeManager",0,0);

  // cellID decoder
  auto decoder = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*det);
  auto id2secmod = [](int id){ return std::pair<int,int>(id&0x7,id>>3); }; // FIXME: use `decoder`

  // dRICH handle
  const string rich_name = "DRICH";
  auto det_rich = det->detector(rich_name);
  auto pos_rich = det_rich.placement().position();

  // start auxfile
  auto irt_auxfile = new TFile(irt_auxfile_name.c_str(),"RECREATE");
  auto irt_geo = new CherenkovDetectorCollection();
  auto irt_det = irt_geo->AddNewDetector(rich_name.c_str());


  // set IRT container volume
  // FIXME: Z-location does not really matter here, right?; but Z-axis orientation does;
  // FIXME: have no connection to GEANT G4LogicalVolume pointers; however all is needed 
  // is to make them unique so that std::map works internally; resort to using integers, 
  // who cares; material pointer can seemingly be '0', and the effective refractive index 
  // for all radiators will be assigned at the end by hand; FIXME: should assign it on 
  // per-photon basis, at birth, like standalone GEANT code does;
  int nSectors = 6;
  TVector3 nx(1,0,0);
  TVector3 ny(0,-1,0);
  for(int isec=0; isec<nSectors; isec++) { // FIXME: do we need a sector loop? probably not...
    irt_geo->SetContainerVolume(
        irt_det, "GasVolume", isec,
        (G4LogicalVolume*)(0x0), 0, new FlatSurface(TVector3(0,0,0), nx, ny)
        );
  };


  //////////////////////////////////
  // begin legacy juggler geosvc code
  //

  /*
  

  // FIXME: Get access to the readout structure decoder
  // irt_det->SetReadoutCellMask( ... )

  // set IRT sensors // FIXME: '0' stands for the unknown (and irrelevant) G4LogicalVolume;
  auto pd = new CherenkovPhotonDetector(0, 0);
  irt_geo->AddPhotonDetector(irt_det, 0, pd);
  
  // loop over RICH detector elements
  bool radClosed = false;
  for(auto const& [richDEname, richDE] : det_rich.children()) {

    // get element attributes
    //if(debug_geosvc) info() << "FOUND RICH element: " << richDEname << endmsg;
    auto dePos = pos_rich + richDE.placement().position();
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
          auto aerogelSurf = new FlatSurface( (1/mm)*TVector3(0,0,dePos.z()), nx, ny);
          irt_geo->AddFlatRadiator(irt_det, "Aerogel", isec, (G4LogicalVolume*)(0x1), 0, aerogelSurf, thickness/mm);
        } else { // elif filter
          auto filterSurf = new FlatSurface( (1/mm)*TVector3(0,0,dePos.z()), nx, ny); // NOTE: there is an airgap in geometry
          irt_geo->AddFlatRadiator(irt_det, "Filter", isec, (G4LogicalVolume*)(0x2), 0, filterSurf, thickness/mm);
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
      irt_det->AddOpticalBoundary(isec, new OpticalBoundary(irt_det->GetContainerVolume(), mirrorSurf, false));
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

      // FIXME: why not `dePos.x(), dePos.y(), dePos.z()`? why the orientation `nx.cross(ny)`, rather than along sphere radius?
      auto sensorSurf = new FlatSurface( (1/mm)*TVector3(0.0, 0.0, dePos.z()), nx, ny);
      irt_det->CreatePhotonDetectorInstance(0, pd, ielem, sensorSurf);

      // close IRT gas radiator by hand (once) // FIXME: why don't we do this after this DetElement loop?
      if(!radClosed && zDirection==-1) { // FIXME: needs to be done for dRICH too
        irt_det->GetRadiator("GasVolume")->m_Borders[0].second = dynamic_cast<ParametricSurface*>(sensorSurf);
        radClosed = true;
      }
    }

    */
  irt_auxfile->Close();
}
