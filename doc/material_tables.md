# Generation of Material Tables

Guide to generate material property tables for XML files for `DD4hep`.

1. Build executables from source in `src/` by running `make`.
2. Execute `bin/generate_material_tables`. This will produce XML nodes for each
   material and optical-surface property table, which can be copied into any
   relevant XML files. Plots of the property tables will also be produced.
3. Extrapolate tables to broader ranges using `scripts/extrapolate_material_tables.py`

### Code
- [`src/g4dRIChOptics.hh`](../src/g4dRIChOptics.hh): Common class hierarchy for
  dRICH material optical properties, originally from
  [cisbani/dRICh](https://github.com/cisbani/dRICh)
- [`text/drich-materials.txt`](../text/drich-materials.txt): Geant4 text-file
  description of materials, needed along with `src/g4dRIChOptics.hh` for the
  generation of property tables
- [`src/generate_material_tables.cpp`](../src/generate_material_tables.cpp):
  Executable for generating the XML nodes and plots
- [`src/surfaceEnums.h`](../src/surfaceEnums.h): helper methods for Geant4 surface
  properties
