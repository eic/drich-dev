#include <cstdlib>
#include <iostream>
#include <fmt/format.h>

#include "g4dRIChOptics.hh"

// ===========================================================================
template<class MAT> class MaterialTable {
  public:
    MAT *mpt;
    MaterialTable(MAT *mpt_) : mpt(mpt_) {};
    ~MaterialTable() { if(mpt!=nullptr) delete mpt; };
    
    // print XML matrices, for `optical_materials.xml`
    void PrintXML(bool isSurface=false, G4String detectorName="DRICH") {
      // function to print a row
      auto PrintRow = [] (int indentation, G4String units="") {
        return [indentation,&units] (G4double energy, G4double value) {
          fmt::print("{:{}}{:<#.5g}{} {:>#.5g}{}\n",
              "",        indentation,
              energy/eV, "*eV",
              value,     units
              );
        };
      };
      if(isSurface) {
        for(const auto& propName : mpt->getMaterialPropertyNames()) {
          fmt::print("{:6}<property name=\"{}\" coldim=\"2\" values=\"\n", "", propName);
          mpt->loopMaterialPropertyTable(propName,PrintRow(8));
          fmt::print("{:8}\"/>\n","");
        }
      } else {
        for(const auto& propName : mpt->getMaterialPropertyNames()) {
          fmt::print("{:4}<matrix name=\"{}__{}_{}\" coldim=\"2\" values=\"\n",
              "", propName, mpt->getMaterialName(), detectorName);
          mpt->loopMaterialPropertyTable(propName,PrintRow(6,"*test"));
          fmt::print("{:6}\"/>\n","");
        }
      }
    } // PrintXML

}; // class MaterialTable

// ===========================================================================

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

  // produce material property tables ///////////////////////

  // aerogel
  MaterialTable Aerogel(new g4dRIChAerogel("Aerogel"));
  Aerogel.mpt->setOpticalParams(aerOptModel);
  Aerogel.PrintXML();

  // acrylic filter
  MaterialTable Acrylic(new g4dRIChFilter("Acrylic"));
  Acrylic.mpt->setOpticalParams(filter_thr);
  Acrylic.PrintXML();

  // gas
  // - C2F6
  MaterialTable C2F6(new g4dRIChGas("C2F6"));
  C2F6.mpt->setOpticalParams();
  C2F6.PrintXML();
  // - C4F10
  MaterialTable C4F10(new g4dRIChGas("C4F10"));
  C4F10.mpt->setOpticalParams();
  C4F10.PrintXML(false,"PFRICH");

  // mirror surface
  MaterialTable MirrorSurface(new g4dRIChMirror("MirrorSurface"));
  MirrorSurface.mpt->setOpticalParams("ciDRICH");
  MirrorSurface.PrintXML(true);

  // photo sensor surface
  MaterialTable SensorSurface(new g4dRIChPhotosensor("SensorSurface"));
  SensorSurface.mpt->setOpticalParams("ciDRICH");
  SensorSurface.PrintXML(true);

} // main
