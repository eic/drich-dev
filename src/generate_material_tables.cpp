#include <cstdlib>
#include <iostream>
#include <fmt/format.h>

#include "g4dRIChOptics.hh"
#include "surfaceEnums.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"

///////////////////////////////////
// SETTINGS
const int    aerOptModel  = 3;      // aerogel optical model used to estimate the refractive Index
const double filter_thr   = 300*nm; // wavelength filter cutoff
const bool   vsWavelength = false;  // if true, make plots vs. wavelength
const std::string xmlOutput = "out/optical_materials_drich.xml";
///////////////////////////////////

// other global vars
std::FILE *xmlFile;

// ===========================================================================
// structures for preferred units
class UnitDef {
  public:
    UnitDef(double divisor_, std::string name_) : divisor(divisor_), name(name_), xml(""), title("") {
      if(name!="") {
        xml   = "*" + name;
        title = " [" + name + "]";
      }
    }
    double divisor;
    std::string name, xml, title;
};


// ===========================================================================
// extends g4dRIChOptics class with plots and XML printing
template<class MAT> class MaterialTable {
  public:
    MAT *mpt;
    std::string name;
    MaterialTable(MAT *mpt_, std::string name_) : mpt(mpt_), name(name_) {
      if(name=="C2F6" or name=="C4F10") PreferredUnits = &PreferredUnits2;
      else PreferredUnits = &PreferredUnits1;
    }
    ~MaterialTable() { if(mpt!=nullptr) delete mpt; };
    
    // print XML matrices, for `optical_materials.xml`
    void PrintXML(bool isSurface=false, G4String detectorName="DRICH") {
      fmt::print(xmlFile,"\n<!-- {:_^60} -->\n\n",name);
      // function to print a row
      auto PrintRow = [] (int indentation, UnitDef units) {
        return [indentation,&units] (G4double energy, G4double value) {
          fmt::print(xmlFile,"{:{}}{:<#.5g}{}   {:>#.5g}{}\n",
              "",                  indentation,
              energy/eV,           "*eV",
              value/units.divisor, units.xml
              );
        };
      };
      if(isSurface) {
        auto surf = mpt->getSurface();
        fmt::print(xmlFile,"{:4}<opticalsurface name=\"{}_{}\" model=\"{}\" finish=\"{}\" type=\"{}\">\n",
            "",
            mpt->getLogicalVName(),
            detectorName,
            surfaceEnum::GetModel(surf),
            surfaceEnum::GetFinish(surf),
            surfaceEnum::GetType(surf)
            );
        for(const auto& propName : mpt->getMaterialPropertyNames()) {
          fmt::print(xmlFile,"{:6}<property name=\"{}\" coldim=\"2\" values=\"\n", "", propName);
          mpt->loopMaterialPropertyTable(propName,PrintRow(8,PreferredUnits->at(propName)));
          fmt::print(xmlFile,"{:8}\"/>\n","");
        }
        fmt::print(xmlFile,"{:4}</opticalsurface>\n","");
      } else {
        for(const auto& propName : mpt->getMaterialPropertyNames()) {
          fmt::print(xmlFile,"{:4}<matrix name=\"{}__{}_{}\" coldim=\"2\" values=\"\n",
              "",
              propName,
              mpt->getMaterialName(),
              detectorName
              );
          mpt->loopMaterialPropertyTable(propName,PrintRow(6,PreferredUnits->at(propName)));
          fmt::print(xmlFile,"{:6}\"/>\n","");
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
        auto units = PreferredUnits->at(propName);
        canv->cd(++pad);
        canv->GetPad(pad)->SetGrid(1,1);
        canv->GetPad(pad)->SetLeftMargin(0.2);
        canv->GetPad(pad)->SetRightMargin(0.15);
        auto graph = new TGraph();
        graph->SetName(TString("graph_"+name+"_"+propName));
        std::string xTitle = vsWavelength ? "wavelength [nm]" : "energy [eV]";
        graph->SetTitle(TString(name+" "+propName+units.title+";"+xTitle));
        int cnt=0;
        auto makeGraph = [&graph,&cnt,&units] (G4double energy, G4double value) {
          auto xval = vsWavelength ? g4dRIChOptics::e2wl(energy)/nm : energy/eV;
          graph->SetPoint(
              cnt++,
              xval,
              value / units.divisor
              );
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

    // sets of preferred units
    const std::map<G4String,UnitDef> *PreferredUnits;
    const std::map<G4String,UnitDef> PreferredUnits1 = {
      { "RINDEX",          UnitDef( 1.,    ""      )},
      { "GROUPVEL",        UnitDef( mm/ns, "mm/ns" )},
      { "RAYLEIGH",        UnitDef( cm,    "cm"    )},
      { "ABSLENGTH",       UnitDef( cm,    "cm"    )},
      { "REFLECTIVITY",    UnitDef( 1.,    ""      )},
      { "REALRINDEX",      UnitDef( 1.,    ""      )},
      { "IMAGINARYRINDEX", UnitDef( 1.,    ""      )},
      { "EFFICIENCY",      UnitDef( 1.,    ""      )}
    };
    const std::map<G4String,UnitDef> PreferredUnits2 = {
      { "RINDEX",          UnitDef( 1.,    ""      )},
      { "GROUPVEL",        UnitDef( mm/ns, "mm/ns" )},
      { "RAYLEIGH",        UnitDef( m,     "m"     )},
      { "ABSLENGTH",       UnitDef( m,     "m"     )},
      { "REFLECTIVITY",    UnitDef( 1.,    ""      )},
      { "REALRINDEX",      UnitDef( 1.,    ""      )},
      { "IMAGINARYRINDEX", UnitDef( 1.,    ""      )},
      { "EFFICIENCY",      UnitDef( 1.,    ""      )}
    };

}; // class MaterialTable

// ===========================================================================

int main(int argc, char** argv) {

  // start XML file
  xmlFile = std::fopen(xmlOutput.c_str(),"w");

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

  // cleanup
  std::fclose(xmlFile);
  fmt::print("\nwrote XML nodes to {}\n\n",xmlOutput);

} // main
