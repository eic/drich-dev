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
using namespace dd4hep;

int main(int argc, char** argv) {

  // compact file name
  string DETECTOR_PATH = string(getenv("DETECTOR_PATH"));
  string DETECTOR      = string(getenv("DETECTOR"));
  if(DETECTOR_PATH.empty() || DETECTOR.empty()) {
    cerr << "ERROR: source environ.sh" << endl;
    return 1;
  }
  string compactFile = DETECTOR_PATH + "/" + DETECTOR + ".xml";

  // irt auxfile name
  string irtAuxFileName = string("geo/irt-drich.root");

  // arguments
  if(argc>1) compactFile    = string(argv[1]);
  if(argc>2) irtAuxFileName = string(argv[2]);

  // full detector handle
  auto det = &(Detector::getInstance());
  det->fromXML(compactFile);

  // dRICH handle
  const string richName = "DRICH";
  auto detRich = det->detector(richName);
  auto posRich = detRich.placement().position();

  // start auxfile
  auto irtAuxFile  = new TFile(irtAuxFileName.c_str(),"RECREATE");
  auto irtGeometry = new CherenkovDetectorCollection();
  auto irtDetector = irtGeometry->AddNewDetector(richName.c_str());

  //// unused
  // auto volman  = det->volumeManager();
  // auto surfman = det->surfaceManager();
  // det->apply("DD4hepVolumeManager",0,0);
  // auto decoder = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*det);
  // auto id2secmod = [](int id){
  //   return std::pair<int,int>(id&0x7,id>>3); // FIXME: use `decoder`
  // };

  // helpers
  // auto starts_with = [](std::string str, const char *pat) {
  //   return str.find(string(pat)) != string::npos; // true if `str` starts with `pat`
  // };
  // auto loop_de = [&detRich,&starts_with](const char *pat, std::function<void(DetElement)> block) {
  //   for(auto const& [de_name, de] : detRich.children()) {
  //     if(starts_with(de_name,pat)) {
  //       cout << "found " << de_name << endl;
  //       block(de);
  //     }
  //   }
  // };
  //   auto get_gasvolMatName = [&gasvolMaterial] (DetElement gasvol) {
  //     gasvolMaterial = gasvol.volume().material().name();
  //   };
  //   loop_de("gasvol",get_gasvolMatName);
  //   cout << gasvolMaterial << endl;

  // begin envelope
  /* FIXME: have no connection to GEANT G4LogicalVolume pointers; however all is needed
   * is to make them unique so that std::map work internally; resort to using integers,
   * who cares; material pointer can seemingly be '0', and effective refractive index
   * for all radiators will be assigned at the end by hand; FIXME: should assign it on
   * per-photon basis, at birth, like standalone GEANT code does;
   */
  auto nSectors       = det->constant<int>("DRICH_RECON_nSectors");
  auto vesselZmin     = det->constant<double>("DRICH_RECON_zmin");
  auto gasvolMaterial = det->constant<string>("DRICH_RECON_gasvolMaterial");
  TVector3 normX(1, 0, 0); // normal vectors
  TVector3 normY(0, -1, 0);
  auto surfEntrance = new FlatSurface((1 / mm) * TVector3(0, 0, vesselZmin), normX, normY);
  for (int isec=0; isec<nSectors; isec++) {
    auto cv = irtGeometry->SetContainerVolume(
        irtDetector,             // Cherenkov detector
        "GasVolume",             // name
        isec,                    // path
        (G4LogicalVolume*)(0x0), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial (inaccessible?)
        surfEntrance             // surface
        );
    cv->SetAlternativeMaterialName(gasvolMaterial.c_str());
  }

  // readout decoder
  // FIXME: copied from `epic:src/DRICH_geo.cpp`; is there an easier way?
  auto        decoder    = desc.readout(readoutName).idSpec().decoder();
  const auto& moduleBits = (*decoder)["module"];
  const auto& sectorBits = (*decoder)["sector"];
  uint64_t    cellMask   = moduleBits.mask() | sectorBits.mask();

  // photon detector
  // - FIXME: args (G4Solid,G4Material) inaccessible?
  CherenkovPhotonDetector* irtPhotonDetector = new CherenkovPhotonDetector(nullptr, nullptr);
  irtDetector->SetReadoutCellMask(cellMask);
  irtGeometry->AddPhotonDetector(
      irtDetector,      // Cherenkov detector
      nullptr,          // G4LogicalVolume (inaccessible?)
      irtPhotonDetector // photon detector
      );

  // aerogel + filter
  /* AddFlatRadiator will create a pair of flat refractive surfaces internally;
   * FIXME: should make a small gas gap at the upstream end of the gas volume;
   * FIXME: do we need a sector loop?
   */
  auto aerogelZpos        = det->constant<double>("DRICH_RECON_aerogelZpos");
  auto aerogelThickness   = det->constant<double>("DRICH_RECON_aerogelThickness");
  auto aerogelMaterial    = det->constant<string>("DRICH_RECON_aerogelMaterial");
  auto filterZpos         = det->constant<double>("DRICH_RECON_filterZpos");
  auto filterThickness    = det->constant<double>("DRICH_RECON_filterThickness");
  auto filterMaterial     = det->constant<string>("DRICH_RECON_filterMaterial");
  auto aerogelFlatSurface = new FlatSurface((1 / mm) * TVector3(0, 0, aerogelZpos), normX, normY);
  auto filterFlatSurface  = new FlatSurface((1 / mm) * TVector3(0, 0, filterZpos), normX, normY);
  for (int isec = 0; isec < nSectors; isec++) {
    auto aerogelFlatRadiator = irtGeometry->AddFlatRadiator(
        irtDetector,             // Cherenkov detector
        "Aerogel",               // name
        isec,                    // path
        (G4LogicalVolume*)(0x1), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial
        aerogelFlatSurface,      // surface
        aerogelThickness / mm    // surface thickness
        );
    auto filterFlatRadiator = irtGeometry->AddFlatRadiator(
        irtDetector,             // Cherenkov detector
        "Filter",                // name
        isec,                    // path
        (G4LogicalVolume*)(0x2), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial
        filterFlatSurface,       // surface
        filterThickness / mm     // surface thickness
        );
    aerogelFlatRadiator->SetAlternativeMaterialName(aerogelMaterial);
    filterFlatRadiator->SetAlternativeMaterialName(filterMaterial);
  }
  // printout(ALWAYS, "IRTLOG", "aerogelZpos = %f cm", aerogelZpos);
  // printout(ALWAYS, "IRTLOG", "filterZpos  = %f cm", filterZpos);
  // printout(ALWAYS, "IRTLOG", "aerogel thickness = %f cm", aerogelThickness);
  // printout(ALWAYS, "IRTLOG", "filter thickness  = %f cm", filterThickness);

  // sector loop
  for (int isec = 0; isec < nSectors; isec++) {

    // mirrors
    auto mirrorRadius = det->constant<double>("DRICH_RECON_mirrorRadius");
    auto mirrorCenterX = det->constant<double>("DRICH_RECON_mirrorCenterX_sec"+std::to_string(isec));
    auto mirrorCenterY = det->constant<double>("DRICH_RECON_mirrorCenterY_sec"+std::to_string(isec));
    auto mirrorCenterZ = det->constant<double>("DRICH_RECON_mirrorCenterZ_sec"+std::to_string(isec));
    auto mirrorSphericalSurface  = new SphericalSurface(
        (1 / mm) * TVector3(mirrorCenterX, mirrorCenterY, mirrorCenterZ), mirrorRadius / mm);
    auto mirrorOpticalBoundary = new OpticalBoundary(
        irtDetector->GetContainerVolume(), // CherenkovRadiator radiator
        mirrorSphericalSurface,            // surface
        false                              // bool refractive
        );
    irtDetector->AddOpticalBoundary(isec, mirrorOpticalBoundary);
    // printout(ALWAYS, "IRTLOG", "");
    // printout(ALWAYS, "IRTLOG", "  SECTOR %d MIRROR:", isec);
    // printout(ALWAYS, "IRTLOG", "    mirror x = %f cm", mirrorCenterX);
    // printout(ALWAYS, "IRTLOG", "    mirror y = %f cm", mirrorCenterY);
    // printout(ALWAYS, "IRTLOG", "    mirror z = %f cm", mirrorCenterZ);
    // printout(ALWAYS, "IRTLOG", "    mirror R = %f cm", mirrorRadius);

    // complete the radiator volume description; this is the rear side of the container gas volume
    irtDetector->GetRadiator("GasVolume")->m_Borders[isec].second = mirrorSphericalSurface;

    // sensors
    // search the detector tree for sensors for this sector
    for(auto const& [de_name, detSensor] : detRich.children()) {
      if(de_name.find("sensor_sec"+std::to_string(isec))!=std::string::npos) {
        auto pvSensor  = detSensor.placement();
        auto posSensor = posRich + pvSensor.position();
        double sensorGlobalPos[3] = {posSensor.x(), posSensor.y(), posSensor.z()}
        auto imodsec = detSensor.id(); // FIXME check this

        // FIXME: is this correct? could this be causing lower than expected NPE?
        // get sensor flat surface normX and normY
        // - ignore vessel transformation, since it is a pure translation
        double sensorLocalNormX[3] = {1.0, 0.0, 0.0};
        double sensorLocalNormY[3] = {0.0, 1.0, 0.0};
        double sensorGlobalNormX[3], sensorGlobalNormY[3];
        pvSensor.LocalToMasterVect(sensorLocalNormX, sensorGlobalNormX);
        pvSensor.LocalToMasterVect(sensorLocalNormY, sensorGlobalNormY);

        // create the optical surface
        auto sensorFlatSurface = new FlatSurface(
            (1 / mm) * TVector3(sensorGlobalPos),
            TVector3(sensorGlobalNormX),
            TVector3(sensorGlobalNormY)
            );
        irtDetector->CreatePhotonDetectorInstance(
            isec,              // sector
            irtPhotonDetector, // CherenkovPhotonDetector
            imodsec,           // copy number
            sensorFlatSurface  // surface
            );
        // if(imod==0) {
        //   printout(ALWAYS, "IRTLOG", "");
        //   printout(ALWAYS, "IRTLOG", "  SECTOR %d SENSORS:", isec);
        // }
        // printout(ALWAYS, "IRTLOG", "    sensor (imodsec,x,y,z) = 0x%08x  %5.2f  %5.2f  %5.2f cm",
        //     imodsec, sensorGlobalPos[0], sensorGlobalPos[1], sensorGlobalPos[2]);
      }
    } // search for sensors

  } // sector loop

  // set refractive indices
  // FIXME: are these (weighted) averages? can we automate this?
  std::map<std::string, double> rIndices;
  rIndices.insert({"GasVolume", 1.0008});
  rIndices.insert({"Aerogel", 1.0190});
  rIndices.insert({"Filter", 1.5017});
  for (auto const& [rName, rIndex] : rIndices) {
    auto rad = irtDetector->GetRadiator(rName.c_str());
    if (rad)
      rad->SetReferenceRefractiveIndex(rIndex);
  }

  // write IRT auxiliary file
  irtGeometry->Write();
  irtAuxFile->Close();


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
      Solid sphPrim;
      auto sphPos = dePos;
      findPrimitive("TGeoSphere",richDE.solid(),sphPrim,sphPos); // get sphere primitive
      auto sph = (Sphere) sphPrim;

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
}
