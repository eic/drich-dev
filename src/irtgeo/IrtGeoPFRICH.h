#pragma once
// bind IRT and DD4hep geometries for the pfRICH

#include "irtgeo/IrtGeo.h"

class IrtGeoPFRICH : public IrtGeo {

  public:
    IrtGeoPFRICH(std::string compactFile_="") : IrtGeo("PFRICH",compactFile_) { DD4hep_to_IRT(); }
    IrtGeoPFRICH(dd4hep::Detector *det_)      : IrtGeo("PFRICH",det_)         { DD4hep_to_IRT(); }
    ~IrtGeoPFRICH() {}

  protected:
    void DD4hep_to_IRT() override;
};
