#pragma once
// bind IRT and DD4hep geometries for the dRICH

#include "irtgeo/IrtGeo.h"

class IrtGeoDRICH : public IrtGeo {

  public:
    IrtGeoDRICH(std::string compactFile_="") : IrtGeo("DRICH",compactFile_) { DD4hep_to_IRT(); }
    IrtGeoDRICH(dd4hep::Detector *det_)      : IrtGeo("DRICH",det_)         { DD4hep_to_IRT(); }
    ~IrtGeoDRICH() {}

  protected:
    void DD4hep_to_IRT() override;
};
