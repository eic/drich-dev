#include <cstdlib>
#include <iostream>
#include <fmt/format.h>

#include "g4dRIChOptics.hh"

using std::string;

int main(int argc, char** argv) {

  /*
  class g4dRIChOptics
  class g4dRIChAerogel     : public g4dRIChOptics
  class g4dRIChFilter      : public g4dRIChOptics
  class g4dRIChGas         : public g4dRIChOptics
  class g4dRIChMirror      : public g4dRIChOptics
  class g4dRIChPhotosensor : public g4dRIChOptics
  */

  // settings
  const int    aerOptModel = 3;      // aerogel optical model used to estimate the refractive Index
  const double filter_thr  = 300*nm; // wavelength filter cutoff

  // - aerogel
  auto aeroPO = new g4dRIChAerogel("ciDRICHaerogelMat");
  aeroPO->setOpticalParams(aerOptModel);
  /*
  // - acrylic filter
  fmt::print("[+] Acrylic Wavelength Threshold : {} nm\n",filter_thr/nm);
  auto acryPO = new g4dRIChFilter("ciDRICHfilterMat");
  acryPO->setOpticalParams(filter_thr);
  // - gas radiator
  auto gasPO = new g4dRIChGas("ciDRICHgasMat");
  gasPO->setOpticalParams();
  // - photo sensors
  auto photoSensor = new g4dRIChPhotosensor("ciDRICHpsst"); 
  photoSensor->setOpticalParams("ciDRICH");
  // - mirror (similar to photosensor, but different params)
  auto mirror = new g4dRIChMirror("ciDRICHmirror"); 
  mirror->setOpticalParams("ciDRICH");
  */
  

}
