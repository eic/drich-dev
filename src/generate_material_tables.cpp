#include <cstdlib>
#include <iostream>
#include <fmt/format.h>

#include "g4dRIChOptics.hh"
#include "surfaceEnums.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"

// ===========================================================================
template<class MAT> class MaterialTable {
  public:
    MAT *mpt;
    std::string name;
    MaterialTable(MAT *mpt_, std::string name_) : mpt(mpt_), name(name_) {};
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
        auto surf = mpt->getSurface();
        fmt::print("{:4}<opticalsurface name=\"{}_{}\" model=\"{}\" finish=\"{}\" type=\"{}\">\n",
            "",
            mpt->getLogicalVName(),
            detectorName,
            surfaceEnum::GetModel(surf),
            surfaceEnum::GetFinish(surf),
            surfaceEnum::GetType(surf)
            );
        for(const auto& propName : mpt->getMaterialPropertyNames()) {
          fmt::print("{:6}<property name=\"{}\" coldim=\"2\" values=\"\n", "", propName);
          mpt->loopMaterialPropertyTable(propName,PrintRow(8));
          fmt::print("{:8}\"/>\n","");
        }
        fmt::print("{:4}</opticalsurface>\n","");
      } else {
        for(const auto& propName : mpt->getMaterialPropertyNames()) {
          fmt::print("{:4}<matrix name=\"{}__{}_{}\" coldim=\"2\" values=\"\n",
              "",
              propName,
              mpt->getMaterialName(),
              detectorName
              );
          mpt->loopMaterialPropertyTable(propName,PrintRow(6,"*test"));
          fmt::print("{:6}\"/>\n","");
        }
      }
    } // PrintXML

    // draw plots of material property tables
    void DrawPlots() {
      TString canvName("materialProperties_"+name);
      TString pngName("out/"+canvName+".png");
      int ncols = 2;
      int nrows = 1+(mpt->getMaterialPropertyTableSize()-1)/ncols;
      if(nrows==1) ncols=mpt->getMaterialPropertyTableSize();
      auto canv = new TCanvas(canvName,canvName,ncols*800,nrows*600);
      canv->Divide(ncols,nrows);
      int pad=0;
      for(const auto& propName : mpt->getMaterialPropertyNames()) {
        fmt::print("Plotting property {}\n", propName);
        canv->cd(++pad);
        canv->GetPad(pad)->SetGrid(1,1);
        canv->GetPad(pad)->SetLeftMargin(0.2);
        canv->GetPad(pad)->SetRightMargin(0.15);
        auto graph = new TGraph();
        graph->SetName(TString("graph_"+name+"_"+propName));
        graph->SetTitle(TString(name+" "+propName+" [units];wavelength [nm]"));
        int cnt=0;
        auto makeGraph = [&graph,&cnt] (G4double energy, G4double value) {
          auto wavelength = g4dRIChOptics::e2wl(energy);
          graph->SetPoint(cnt++,wavelength/nm,value);
        };
        mpt->loopMaterialPropertyTable(propName,makeGraph);
        graph->SetMarkerStyle(kFullCircle);
        graph->SetMarkerColor(kAzure);
        graph->GetXaxis()->SetLabelSize(0.05);
        graph->GetYaxis()->SetLabelSize(0.05);
        graph->GetXaxis()->SetTitleOffset(1.3);
        graph->Draw("AP");
      }
      canv->SaveAs(pngName); 
    }

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
  MaterialTable Aerogel(new g4dRIChAerogel("Aerogel"),"Aerogel");
  Aerogel.mpt->setOpticalParams(aerOptModel);
  Aerogel.PrintXML();
  Aerogel.DrawPlots();

  // acrylic filter
  MaterialTable Acrylic(new g4dRIChFilter("Acrylic"),"Acrylic");
  Acrylic.mpt->setOpticalParams(filter_thr);
  Acrylic.PrintXML();
  Acrylic.DrawPlots();

  // gas
  // - C2F6
  MaterialTable C2F6(new g4dRIChGas("C2F6"),"C2F6");
  C2F6.mpt->setOpticalParams();
  C2F6.PrintXML();
  C2F6.DrawPlots();
  // - C4F10
  MaterialTable C4F10(new g4dRIChGas("C4F10"),"C4F10");
  C4F10.mpt->setOpticalParams();
  C4F10.PrintXML(false,"PFRICH");
  C4F10.DrawPlots();

  // mirror surface
  MaterialTable MirrorSurface(new g4dRIChMirror("MirrorSurface"),"MirrorSurface");
  MirrorSurface.mpt->setOpticalParams("ciDRICH");
  MirrorSurface.PrintXML(true);
  MirrorSurface.DrawPlots();

  // photo sensor surface
  MaterialTable SensorSurface(new g4dRIChPhotosensor("SensorSurface"),"SensorSurface");
  SensorSurface.mpt->setOpticalParams("ciDRICH");
  SensorSurface.PrintXML(true);
  SensorSurface.DrawPlots();

} // main
