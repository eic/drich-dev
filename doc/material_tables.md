# Generation of Material Tables

Guide to generate material property tables for XML files for `DD4hep`.

## Procedure

1. Build executables from source in `src/` by running `make`.
2. Execute `bin/generate_material_tables`. This will produce XML nodes for each
   material and optical-surface property table, which can be copied into any
   relevant XML files. Plots of the property tables will also be produced, but note
   that step 3 will extend or override these plots; always refer to the plots from
   step 3 in place of those from step 2, since in some cases, the step 2 tables are
   out of date
3. Extrapolate tables to broader ranges using `scripts/extrapolate_material_tables.py`;
   these tables should be used in place of the ones from step 2

## Code
- [`src/g4dRIChOptics.hh`](../src/g4dRIChOptics.hh): Common class hierarchy for
  dRICH material optical properties, originally from
  [cisbani/dRICh](https://github.com/cisbani/dRICh)
- [`text/drich-materials.txt`](../text/drich-materials.txt): Geant4 text-file
  description of materials, needed along with `src/g4dRIChOptics.hh` for the
  generation of property tables
- [`src/generate_material_tables.cpp`](../src/generate_material_tables.cpp):
  Executable for generating the XML nodes and plots
- [`scripts/extrapolate_material_tables.py`](../scripts/extrapolate_material_tables.py):
  Extrapolate material property tables beyond `g4dRIChOptics`
- [`src/surfaceEnums.h`](../src/surfaceEnums.h): helper methods for Geant4 surface
  properties

# Material Property Tables Status

## Aerogel
### Material
- using CLAS12 experimental points rescaled by Alessio/GEMC, in range 200 to 660 nm
- needed to do curve fits for extrapolation
- [x] **EXTRAPOLATION**: refractive index
  - fit to 2nd order Sellmeier function
  - upper limit 660 nm extrapolated to 1000 nm
- [x] **EXTRAPOLATION**: absorption length
  - linear fit to 350 nm and above only
  - upper limit 660 nm extrapolated to 1000 nm
- [x] **EXTRAPOLATION**: Rayleigh scattering length
  - fit to `lambda^4` dependence
  - upper limit 660 nm extrapolated to 1000 nm
### Surface
- [ ] **TODO**

## Airgap
- using common `AirOptical`
- no effect switching to `C2F6` (which disables it)
- no extrapolation

## Acrylic Filter
### Material
- using (adjustable) cutoff of 300 nm
- no effect switching to `C2F6` (which disables it)
- no extrapolation
### Surface
- [ ] **TODO**

## Gas
### Material
- currently 200-700nm, 10 points
- straightforward to adjust in Evaristo's code, `g4dRIChOptics.hh`
- [x] **EXTRAPOLATION**: refractive index
  - extrapolated to 200-1000nm, 16 points
  - done for both C2F6 and C4F10
- [x] **EXTRAPOLATION**: absorption length
  - is constant
  - same range as refractive index
  - done for both C2F6 and C4F10
- [ ] **TODO**: use Rayleigh scattering length?
  - also is constant, but not used in `epic`
### Surface
- [ ] **TODO**?

## Mirror
### Material
- same as filter material
- [ ] **TODO**: update to a more realistic material
### Surface
- model exists from Evaristo's code, but we are not using this since reflectivity seems too low
- instead, using constant 0.9 reflectivity for all wavelengths: 1-7 eV = 177-1240nm
- no extrapolation, since wavelength range is large enough
- [ ] **TODO**: update to a more realistic model or measurement

## Sensors
### Material
- Material is currently `AirOptical`, to correctly model the gas/sensor optical boundary
- [ ] **TODO**: do this more correctly
### Surface
- table `EFFICIENCY` is set to constant 1 for all wavelengths 1-7 eV = 177-1240nm
- actual quantum efficiency is applied downstream in reconstruction
