# Aerogel
## Material
- using CLAS12 experimental points rescaled by Alessio/GEMC
  - 200 to 660 nm
  - cannot easily extrapolate, but may be possible with (piecewise) curve fit
## Surface
- TODO

# Airgap
- using common `AirOptical`
- no effect switching to `C2F6`

# Acrylic Filter
## Material
- using (adjustable) cutoff of 300 nm
- can extrapolate with a curve fit
## Surface
- TODO

# Gas
## Material
- straightforward to extrapolate
  - what wavelength range?
  - how many points?
  - looks weird below ~100 nm?
## Surface
- not needed?

# Mirror
## Material
- same as filter material
## Surface
- 0.9 reflectivity for all wavelengths
- extrapolate to larger range?

# Sensors
## Material
- `AirOptical`
## Surface
- TODO (but `EFFICIENCY` is set to 1.0 for all wavelength)
- extrapolate to larger range?
