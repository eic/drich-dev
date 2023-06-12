# Aerogel
## Material
- using CLAS12 experimental points rescaled by Alessio/GEMC, in range 200 to 660 nm
- needed to do curve fits for extrapolation
- [x] **CHANGE**: extrapolate refractive index
  - fit to 2nd order Sellmeier function
  - upper limit 660 nm extrapolated to 1000 nm
- [x] **CHANGE**: extrapolate absorption length
  - linear fit to 350 nm and above only
  - upper limit 660 nm extrapolated to 1000 nm
- [x] **CHANGE**: extrapolate Rayleigh scattering length
  - fit to `lambda^4` dependence
  - upper limit 660 nm extrapolated to 1000 nm
## Surface
- [ ] **TODO**

# Airgap
- using common `AirOptical`
- no effect switching to `C2F6` (which disables it)
- no changes

# Acrylic Filter
## Material
- using (adjustable) cutoff of 300 nm
- no effect switching to `C2F6` (which disables it)
- no changes
## Surface
- [ ] **TODO**

# Gas
## Material
- currently 200-700nm, 10 points
- straightforward to adjust in Evaristo's code, `g4dRIChOptics.hh`
- [x] **CHANGE**: extrapolate refractive index
  - extrapolated to 200-1000nm, 16 points
  - done for both C2F6 and C4F10
- [x] **CHANGE**: extrapolate absorption length
  - is constant
  - same range as refractive index
  - done for both C2F6 and C4F10
- [ ] **TODO**: use Rayleigh scattering length?
  - also is constant, but not used in `epic`
## Surface
- [ ] **TODO**?

# Mirror
## Material
- same as filter material
- [ ] **TODO**: update to a more realistic material
## Surface
- model exists from Evaristo's code, but we are not using this since reflectivity seems too low
- instead, using constant 0.9 reflectivity for all wavelengths: 1-7 eV = 177-1240nm
- no changes at this time, since wavelength range is large enough
- [ ] **TODO**: update to a more realistic model or measurement

# Sensors
## Material
- Material is currently `AirOptical`, to correctly model the gas/sensor optical boundary
- [ ] **TODO**: do this more correctly
## Surface
- table `EFFICIENCY` is set to constant 1 for all wavelengths 1-7 eV = 177-1240nm
- actual quantum efficiency is applied downstream in reconstruction
- [x] **CHANGE**: minor change, extrapolating the end points of the QE curve down to zero
  - done by eye, adding 2 new points with QE=0 at 315 nm and at 1000 nm
