# dRICH-dev
Scripts for ATHENA dRICH and IRT development 

ATHENA Software is modular: see [the flowchart overview](docDiagram.pdf) for general guidance of the modules relevant for RICH development. It shows their dependences, calls, and data flow.

See [MR.md](MR.md) for information about the current development branches and active merge requests

## Dependencies
- install EIC Software container:
  - `opt/update.sh` will install or update the EIC Software container automatically
    - execute `opt/eic-shell` to start the container
    - execute `echo $ATHENA_PREFIX`
      - it should be `./opt/local` unless you changed it
      - this is the prefix that will be used for builds of ATHENA Software modules (edit `environ.sh` to change prefixes)
    - depending on your setup, you may want or need to pass additional options; see for example `opt/update.arcturus.sh`
  - alternatively, follow the [eic_container documentation](https://eicweb.phy.anl.gov/containers/eic_container)
    - `opt/update.sh` is a wrapper for this procedure for storing the image and prefix locally in `opt/` 
- obtain ATHENA Software modules, either clone or symlink the repositories to the specified paths
  - modules:
    - [detectors/ip6](https://eicweb.phy.anl.gov/EIC/detectors/ip6) to `./ip6`
    - [detectors/athena](https://eicweb.phy.anl.gov/EIC/detectors/athena) to `./athena`
    - [irt](https://eicweb.phy.anl.gov/EIC/irt) to `./irt`
    - [eicd](https://eicweb.phy.anl.gov/EIC/eicd) to `./eicd`
    - [Project Juggler](https://eicweb.phy.anl.gov/EIC/juggler) to `./juggler`
    - [benchmarks/reconstruction_benchmarks](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks) to `./reconstruction_benchmarks`
    - suggestion: clone with SSH:
    ```
    git clone git@eicweb.phy.anl.gov:EIC/detectors/ip6.git
    git clone git@eicweb.phy.anl.gov:EIC/detectors/athena.git
    git clone git@eicweb.phy.anl.gov:EIC/irt.git
    git clone git@eicweb.phy.anl.gov:EIC/eicd.git
    git clone git@eicweb.phy.anl.gov:EIC/juggler.git
    git clone git@eicweb.phy.anl.gov:EIC/benchmarks/reconstruction_benchmarks.git
    ```
  - follow directions below to build each module
  - alternatively, follow the [IRT Readme](https://eicweb.phy.anl.gov/EIC/irt) for module setup and building
    - the IRT Readme describes a setup that was based off the setup in this repository, but with some minor differences such as prefix paths

## Environment
- execute `source environ.sh`
  - this file contains several env vars needed by many scripts
  - make sure `$BUILD_NCPUS` is set correctly
    - this will be the number of CPUs used for multi-threaded building, and running Juggler multi-threaded
    - reduce it, if you prefer
    - memory-hungry builds will be built single-threaded
  - edit other variables if you want, e.g., for specifying alternate prefixes; you can find documentation for many variables in the corresponding module repositories

## Building Modules
- you must be in the EIC container (`opt/eic-shell`) and have environment variables set (`source environ.sh`)
- build each repository, one-by-one, in order of dependences (see [flowchart](docDiagram.pdf) dependency graph)
- instructions for the `reconstruction_benchmarks` repository are below
- build scripts, in recommended order:
```
./buildEICD.sh
./buildIRT.sh
./buildIP6.sh
./buildATHENA.sh
./buildJuggler.sh
```

### Recommendations and Troubleshooting
- execute `./rebuildAll.sh` to quickly rebuild all repositories, in order of dependences; this is useful when you switch branches in any of the repositories
- be mindful of which branch you are on in each repository, especially if you have several active merge requests
  - for example, `irt` requires the new `eicd` components and datatypes, which at the time of writing this have not been merged to `eicd` `master`
  - use `./checkBranches.sh` to quickly check which branches you are on in all repositories
  - use `./checkStatus.sh` to run `git status` in each repository, which is useful during active development
- for clean builds, you can generally pass the word `clean` to any build script (you can also do `./rebuildAll.sh clean` to clean build everything)
- most build scripts will run `cmake --build` multi-threaded
  - the `$BUILD_NCPUS` environment variable should be set to the number of CPUs you want to build with (see `environ.sh`)
  - careful, some module builds consume a lot of memory (Juggler); the build scripts will force single-threaded building for such cases

### Benchmarks Setup
before running benchmarks, you must setup the common benchmarks:
```
pushd reconstruction_benchmarks
git clone git@eicweb.phy.anl.gov:EIC/benchmarks/common_bench.git setup
source setup/bin/env.sh
./setup/bin/install_common.sh
popd
source environ.sh
```
- these directions are similar to those in the [reconstruction_benchmarks Readme](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks/-/blob/master/README.md), but with some minor differences for our current setup (e.g., there is no need to build another detector in `reconstruction_benchmarks/.local`)
- the `source environ.sh` step will now set additional environment variables, since you now have common benchmarks installed
- there is no need to repeat this setup procedure, unless you want to start from a clean slate or update the common benchmarks

## Execution

### Geometry
- execute `./runDDwebDisplay.sh` to produce the geometry `root` file
  - by default, it will use the compact file for the full ATHENA detector
  - run `./runDDwebDisplay.sh d` to run on dRICH only
  - run `./runDDwebDisplay.sh p` to run on pfRICH only
- open the resulting ROOT file in `jsroot` geoviewer, using either:
  - [ANL hosted instance](https://eic.phy.anl.gov/geoviewer/)
  - [CERN hosted instance](https://root.cern/js/)
  - a locally hosted instance
- browse the ROOT file geometry tree in the sidebar on the left:
  ```
  detector_geometry.root
  └── default
      └── world_volume
          ├── ...
          ├── DRICH
          ├── PFRICH
          └── ...
  ```
  - right click on desired component, then click `Draw`
  - default projection is perspective, but if you need to check alignment, change to orthographic projection
    - right click -> show controls -> advanced -> orthographic camera
    - square your browser window aspect ratio, since the default aspect ratio is whatever your browser window is
  - more documentation found on [jsroot website](https://root.cern/js/)
- check for overlaps
  - typically more efficient to let the CI do this
  - call `./overlapCheck.sh` to run a local check
    - one check faster and less accurate, the other is slower and more accurate
- use `./searchCompactParams.sh [PATTERN]` to quickly obtain the value of any parameter in the compact files
  - for example, `./searchCompactParams.sh RICH` to get all RICH variables
  - the search pattern is case sensitive
  - this script is just a wrapper for `npdet_info`, run `npdet_info -h` for further usage

### Simulation and Reconstruction Benchmarks
- use `./runBenchmark.sh` to run the simulation and subsequent reconstruction
  - this is a wrapper for `reconstruction_benchmarks/benchmarks/rich/run_irt.sh`, which is executed by the CI
    - this script runs `npsim` and `juggler`
  - see also `reconstruction_benchmarks/benchmarks/rich/config.yml` for the commands used by the CI
  - it is practical to edit this wrapper script during development, for testing purposes; this is why several lines are commented out

#### Local Scripts
There are some local scripts to aid in simulation development; some of them have been copied to the `reconstruction_benchmarks` repository, and may be more up-to-date there.
- `simulate.py`: runs `npsim` with settings for the dRICH and pfRICH
  - run with no arguments for usage
  - basically copied to `reconstruction_benchmarks`, but stored here as well for backup
- `drawHits.cpp`
  - reads simulation output and draws raw hit positions and number of hits vs. momentum
  - build with `make`, execute with `./drawHits.exe [simulation_output_file]`
  - bacially copied to `reconstruction_benchmarks`
  - specific for dRICH; for pfRICH version, see `pfrich/`
- `drawSegmentation.cpp`
  - reads simulation output and draws the hits within sensor pixels, which is useful for checking mapping
  - relies on `text/sensorLUT.dat`, which must be up-to-date
    - you can produce a new version of this file by uncommenting relevant lines in `athena/src/DRICH_geo.cpp` (search for `generate LUT`), and running something like `./rebuildAll.sh && ./runDDwebDisplay.sh`
  - build with `make`, execute with `./drawSegmentation.exe [simulation_output_file]`
  - specific for dRICH; for pfRICH version, see `pfrich/`

### Miscellaneous
- `makeDocumentation.sh`: calls script for auto-documentation from compact tags, outputs in `./doc`
- `deprecated/` contains some old scripts which may be helpful
