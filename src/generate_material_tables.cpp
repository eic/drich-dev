#include <cstdlib>
#include <iostream>
#include <fmt/format.h>

#include "g4dRIChOptics.hh"

using std::string;

int main(int argc, char** argv) {

  // settings
  const int    aerOptModel = 3;      // aerogel optical model used to estimate the refractive Index
  const double filter_thr  = 300*nm; // wavelength filter cutoff

  // build detector by text file
  fmt::print("[+] read model text file\n");
  G4tgbVolumeMgr *volmgr = G4tgbVolumeMgr::GetInstance();
  auto model_file = G4String("text/drich-materials.txt");
  volmgr->AddTextFile(model_file);
  fmt::print("[+] construct detector from text file\n");
  G4VPhysicalVolume *vesselPhysVol = volmgr->ReadAndConstructDetector();
  fmt::print("[+] done construction\n");

  // - aerogel
  auto aeroPO = new g4dRIChAerogel("aerogel");
  aeroPO->setOpticalParams(aerOptModel);
  // - acrylic filter
  auto acryPO = new g4dRIChFilter("filter");
  acryPO->setOpticalParams(filter_thr);
  // - gas radiator options
  std::vector<G4String> gasMaterials = {"C2F6","C4F10"};
  for(auto gasMaterial : gasMaterials) {
    auto gasPO = new g4dRIChGas(gasMaterial);
    gasPO->setOpticalParams();
  }
  // - mirror (similar to photosensor, but different params)
  auto mirror = new g4dRIChMirror("mirrorVol");
  mirror->setOpticalParams("ciDRICH");
  // - photo sensors
  auto photoSensor = new g4dRIChPhotosensor("sensorVol"); 
  photoSensor->setOpticalParams("ciDRICH");
}
