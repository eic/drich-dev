// map Geant4 surface enum values to strings
//

#include <string>
#include <vector>
#include "Geant4/G4OpticalSurface.hh"

namespace surfaceEnum {

  // reverse enumerators
  // FIXME: maybe there is a better way from within Geant4, but after a first look,
  // there isn't one; these string vectors have been adapted from enums in
  // `G4*Surface*.hh` headers
  const std::vector<std::string> Type = {
    "dielectric_metal",
    "dielectric_dielectric",
    "dielectric_LUT",
    "dielectric_LUTDAVIS",
    "dielectric_dichroic",
    "firsov",
    "x_ray"
  };
  const std::vector<std::string> Model = {
    "glisur",
    "unified",
    "LUT",
    "DAVIS",
    "dichroic"
  };
  const std::vector<std::string> Finish = {
    "polished",
    "polishedfrontpainted",
    "polishedbackpainted",
    "ground",
    "groundfrontpainted",
    "groundbackpainted",
    "polishedlumirrorair",
    "polishedlumirrorglue",
    "polishedair",
    "polishedteflonair",
    "polishedtioair",
    "polishedtyvekair",
    "polishedvm2000air",
    "polishedvm2000glue",
    "etchedlumirrorair",
    "etchedlumirrorglue",
    "etchedair",
    "etchedteflonair",
    "etchedtioair",
    "etchedtyvekair",
    "etchedvm2000air",
    "etchedvm2000glue",
    "groundlumirrorair",
    "groundlumirrorglue",
    "groundair",
    "groundteflonair",
    "groundtioair",
    "groundtyvekair",
    "groundvm2000air",
    "groundvm2000glue",
    "Rough_LUT",
    "RoughTeflon_LUT",
    "RoughESR_LUT",
    "RoughESRGrease_LUT",
    "Polished_LUT",
    "PolishedTeflon_LUT",
    "PolishedESR_LUT",
    "PolishedESRGrease_LUT",
    "Detector_LUT"
  };

  // accessors
  static std::string GetType(G4OpticalSurface   *surf) { return Type[int(surf->GetType())];     }
  static std::string GetModel(G4OpticalSurface  *surf) { return Model[int(surf->GetModel())];   }
  static std::string GetFinish(G4OpticalSurface *surf) { return Finish[int(surf->GetFinish())]; }
}
