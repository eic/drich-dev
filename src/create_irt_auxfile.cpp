#include <cstdlib>
#include <iostream>
#include <fmt/format.h>

// ROOT
#include "TSystem.h"
#include "TFile.h"

// DD4Hep
#include "DD4hep/Detector.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Printout.h"

// IRT
#include "CherenkovDetectorCollection.h"
#include "CherenkovPhotonDetector.h"
#include "CherenkovRadiator.h"
#include "OpticalBoundary.h"
#include "ParametricSurface.h"

using std::string;
using namespace dd4hep;

int main(int argc, char** argv) {

  // compact file name
  string DETECTOR_PATH(getenv("DETECTOR_PATH"));
  string DETECTOR(getenv("DETECTOR"));
  if(DETECTOR_PATH.empty() || DETECTOR.empty()) {
    fmt::print(stderr,"ERROR: source environ.sh\n");
    return 1;
  }
  string compactFile = DETECTOR_PATH + "/" + DETECTOR + ".xml";

  // irt auxfile name
  string irtAuxFileName = string("geo/irt-drich.root");

  // constant names
  const string richName    = "DRICH";
  const string readoutName = "DRICHHits";

  // arguments
  if(argc>1) compactFile    = string(argv[1]);
  if(argc>2) irtAuxFileName = string(argv[2]);

  // full detector handle
  auto det = &(Detector::getInstance());
  det->fromXML(compactFile);

  // dRICH handle
  auto detRich = det->detector(richName);
  auto posRich = detRich.placement().position();

  // start auxfile
  auto irtAuxFile  = new TFile(irtAuxFileName.c_str(),"RECREATE");
  auto irtGeometry = new CherenkovDetectorCollection();
  auto irtDetector = irtGeometry->AddNewDetector(richName.c_str());

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

  // photon detector
  // - FIXME: args (G4Solid,G4Material) inaccessible?
  auto cellMask = uint64_t(std::stoull(det->constant<string>("DRICH_RECON_cellMask")));
  CherenkovPhotonDetector* irtPhotonDetector = new CherenkovPhotonDetector(nullptr, nullptr);
  irtDetector->SetReadoutCellMask(cellMask);
  irtGeometry->AddPhotonDetector(
      irtDetector,      // Cherenkov detector
      nullptr,          // G4LogicalVolume (inaccessible?)
      irtPhotonDetector // photon detector
      );
  printout(ALWAYS, "IRTLOG", "cellMask = 0x%X", cellMask);

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
  auto filterFlatSurface  = new FlatSurface((1 / mm) * TVector3(0, 0, filterZpos),  normX, normY);
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
    aerogelFlatRadiator->SetAlternativeMaterialName(aerogelMaterial.c_str());
    filterFlatRadiator->SetAlternativeMaterialName(filterMaterial.c_str());
  }
  printout(ALWAYS, "IRTLOG", "aerogelZpos = %f cm", aerogelZpos);
  printout(ALWAYS, "IRTLOG", "filterZpos  = %f cm", filterZpos);
  printout(ALWAYS, "IRTLOG", "aerogel thickness = %f cm", aerogelThickness);
  printout(ALWAYS, "IRTLOG", "filter thickness  = %f cm", filterThickness);

  // sector loop
  for (int isec = 0; isec < nSectors; isec++) {
    string secName = "sec" + std::to_string(isec);

    // mirrors
    auto mirrorRadius = det->constant<double>("DRICH_RECON_mirrorRadius");
    Position mirrorCenter(
      det->constant<double>("DRICH_RECON_mirrorCenterX_"+secName),
      det->constant<double>("DRICH_RECON_mirrorCenterY_"+secName),
      det->constant<double>("DRICH_RECON_mirrorCenterZ_"+secName)
      );
    auto mirrorSphericalSurface  = new SphericalSurface(
        (1 / mm) * TVector3(mirrorCenter.x(), mirrorCenter.y(), mirrorCenter.z()), mirrorRadius / mm);
    auto mirrorOpticalBoundary = new OpticalBoundary(
        irtDetector->GetContainerVolume(), // CherenkovRadiator radiator
        mirrorSphericalSurface,            // surface
        false                              // bool refractive
        );
    irtDetector->AddOpticalBoundary(isec, mirrorOpticalBoundary);
    printout(ALWAYS, "IRTLOG", "");
    printout(ALWAYS, "IRTLOG", "  SECTOR %d MIRROR:", isec);
    printout(ALWAYS, "IRTLOG", "    mirror x = %f cm", mirrorCenter.x());
    printout(ALWAYS, "IRTLOG", "    mirror y = %f cm", mirrorCenter.y());
    printout(ALWAYS, "IRTLOG", "    mirror z = %f cm", mirrorCenter.z());
    printout(ALWAYS, "IRTLOG", "    mirror R = %f cm", mirrorRadius);

    // complete the radiator volume description; this is the rear side of the container gas volume
    irtDetector->GetRadiator("GasVolume")->m_Borders[isec].second = mirrorSphericalSurface;

    // sensor sphere (only used for validation of sensor normals)
    auto sensorSphRadius  = det->constant<double>("DRICH_RECON_sensorSphRadius");
    auto sensorThickness  = det->constant<double>("DRICH_RECON_sensorThickness");
    Position sensorSphCenter(
      det->constant<double>("DRICH_RECON_sensorSphCenterX_"+secName),
      det->constant<double>("DRICH_RECON_sensorSphCenterY_"+secName),
      det->constant<double>("DRICH_RECON_sensorSphCenterZ_"+secName)
      );
    printout(ALWAYS, "IRTLOG", "  SECTOR %d SENSOR SPHERE:", isec);
    printout(ALWAYS, "IRTLOG", "    sphere x = %f cm", sensorSphCenter.x());
    printout(ALWAYS, "IRTLOG", "    sphere y = %f cm", sensorSphCenter.y());
    printout(ALWAYS, "IRTLOG", "    sphere z = %f cm", sensorSphCenter.z());
    printout(ALWAYS, "IRTLOG", "    sphere R = %f cm", sensorSphRadius);

    // sensor modules: search the detector tree for sensors for this sector
    for(auto const& [de_name, detSensor] : detRich.children()) {
      if(de_name.find("sensor_de_"+secName)!=string::npos) {

        // get sensor position
        auto pvSensor  = detSensor.placement();
        auto posSensor = posRich + pvSensor.position();
        double sensorGlobalPos[3] = {posSensor.x(), posSensor.y(), posSensor.z()}; // FIXME: unused
        auto imodsec = detSensor.id();

        // get surface normal
        // FIXME: is this correct? could this be causing lower than expected NPE?
        // get sensor flat surface normX and normY
        // - ignore vessel transformation, since it is a pure translation
        double sensorLocalNormX[3] = {1.0, 0.0, 0.0};
        double sensorLocalNormY[3] = {0.0, 1.0, 0.0};
        double sensorGlobalNormX[3], sensorGlobalNormY[3];
        pvSensor.ptr()->LocalToMasterVect(sensorLocalNormX, sensorGlobalNormX);
        pvSensor.ptr()->LocalToMasterVect(sensorLocalNormY, sensorGlobalNormY);

        // validate sensor position and normal
        Direction radialDir = posSensor - sensorSphCenter; // sensor sphere radius direction
        Direction normXdir, normYdir;
        normXdir.SetCoordinates(sensorGlobalNormX);
        normYdir.SetCoordinates(sensorGlobalNormY);
        auto distSensor2center = sqrt((posSensor-sensorSphCenter).Mag2()); // distance between sensor sphere center and sensor position
        auto normZdir = normXdir.Cross(normYdir); // sensor surface normal
        // - test quantities
        auto testOrtho  = normXdir.Dot(normYdir);           // should be zero, if normX and normY are orthogonal
        auto testRadial = radialDir.Cross(normZdir).Mag2(); // should be zero, if sensor surface normal is parallel to sensor sphere radius
        auto testDist   = abs(distSensor2center-(sensorSphRadius-sensorThickness/2.0)); // should be zero, if sensor position w.r.t. sensor sphere center is correct
        if(abs(testOrtho)>1e-6 || abs(testRadial)>1e-6) {
          printout(FATAL, "IRTLOG",
              "sensor normal is wrong: normX.normY = %f   |radialDir x normZdir|^2 = %f",
              testOrtho,
              testRadial
              );
          return 1;
        }
        if(abs(testDist)>1e-6) {
          printout(FATAL, "IRTLOG",
              "sensor positioning is wrong: dist(sensor, sphere_center) = %f,  sphere_radius = %f,  sensor_thickness = %f,  |diff| = %g\n",
              distSensor2center,
              sensorSphRadius,
              sensorThickness,
              testDist
              );
          return 1;
        }


        // create the optical surface
        auto sensorFlatSurface = new FlatSurface(
            (1 / mm) * TVector3(posSensor.x(), posSensor.y(), posSensor.z()),
            TVector3(sensorGlobalNormX),
            TVector3(sensorGlobalNormY)
            );
        irtDetector->CreatePhotonDetectorInstance(
            isec,              // sector
            irtPhotonDetector, // CherenkovPhotonDetector
            imodsec,           // copy number
            sensorFlatSurface  // surface
            );
        // printout(ALWAYS, "IRTLOG",
        //     "sensor: id=0x%08X pos=(%5.2f, %5.2f, %5.2f) normX=(%5.2f, %5.2f, %5.2f) normY=(%5.2f, %5.2f, %5.2f)",
        //     imodsec,
        //     posSensor.x(), posSensor.y(), posSensor.z(),
        //     normXdir.x(),  normXdir.y(),  normXdir.z(),
        //     normYdir.x(),  normYdir.y(),  normYdir.z()
        //     );
      }
    } // search for sensors

  } // sector loop

  // set refractive indices
  // FIXME: are these (weighted) averages? can we automate this?
  std::map<string, double> rIndices;
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
  fmt::print("\nWrote IRT Aux File: {}\n\n",irtAuxFileName);
}
