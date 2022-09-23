# Generation of Material Tables

Guide to generate material property tables for XML files for `DD4hep`.

1. Build executables from source in `src/` by running `make`.
2. Execute `bin/generate_material_tables`. This will produce XML nodes for each
   material and optical-surface property table, which can be copied into any
   relevant XML files. Plots of the property tables will also be produced.

### Code
- `src/g4dRIChOptics.hh`: Common dRICH optics class, originally from
  [cisbani/dRICh](https://github.com/cisbani/dRICh); this can be used
  standalone for any Geant4 simulations
- `text/drich-materials.txt`: Geant4 text-file description of materials, needed
  along with `src/g4dRIChOptics.hh` for the generation of property tables
- `src/generate_material_tables.cpp`: Executable for generating the XML
  nodes and plots
- `surfaceEnums.h`: helper methods for Geant4 surface properties
