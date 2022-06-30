# dRICH-dev
Resources and Tools for EIC dRICH development 

| **Table of Contents**             |                                         |
| --:                               | ---                                     |
| [Setup](#setup)                   | How to download and build the code      |
| [Implementation](#implementation) | Where to find the code and what it does |
| [Execution](#execution)           | How to run the code                     |
| [Algorithms](#algorithms)         | Documentation for algorithms            |

| **Documentation Links**                        |                                                |
| --:                                            | ---                                            |
| [Flowchart](doc/docDiagram.pdf)                | Diagram of software modules                    |
| [Links](doc/links.md)                          | Collection of dRICH Software and Resources     |
| [Branches and Merge Requests](doc/branches.md) | Active development branches and merge requests |

## Notes
EIC Software is modular: see [the flowchart overview](doc/docDiagram.pdf) for
general guidance of the modules relevant for RICH development. It shows their
dependences, calls, and data flow.

See also [the collection of links](doc/links.md) to various resources and dRICH
implementations.

## Active Branches

Depending on the development, you likely need to change `git` branches for some
of the modules. See the [active branches tables](doc/branches.md) for tables
of branches for varying configurations.

## Notes for ATHENA
- This repository was used for development of the ATHENA dRICH and pfRICH; it
  has since been modified to support the Project Detector of the EIC
- See [doc/athena-branches.md](doc/athena-branches.md) for information about the
  development branches and merge requests that were used for the ATHENA proposal
- It is possible that the pfRICH scripts no longer work, since we now focus on
  the dRICH

---

<a name="setup"></a>
# Setup
- First, clone this `drich-dev` repository
  - If you follow the directions below as is, everything will be installed in
    subdirectories of this repository; you will need a few GB of disk space
  - If you have experience with the ATHENA software stack, you may prefer your
    own set up; in that case, make symlinks to your local `git` repository
    clones, so you can use the scripts in this directory
- Obtain the EIC Software image (`jug_xl`):
  - Run `opt/update.sh` to obtain (or update) the EIC Software image automatically
    - this is just a wrapper of the commonly-used `install.sh` script;
      alternatively, use that script directly by following
      [eic-container documentation](https://eicweb.phy.anl.gov/containers/eic_container)
    - depending on your setup, you may want or need to pass additional options;
      see for example `opt/update.arcturus.sh` (especially if you are low on
      disk space on your `/` partition)
    - the image and builds will be stored in `./opt`
  - execute `opt/eic-shell` to start the container; practically everything below
    must be executed within this container
- obtain EIC Software modules, either clone or symlink the repositories to the
  specified paths:
  - modules:
    - [detectors/ip6](https://eicweb.phy.anl.gov/EIC/detectors/ip6) to `./ip6`
    - [detectors/ecce](https://eicweb.phy.anl.gov/EIC/detectors/ecce) to `./ecce`
    - [irt](https://eicweb.phy.anl.gov/EIC/irt) to `./irt`
    - [eicd](https://eicweb.phy.anl.gov/EIC/eicd) to `./eicd`
    - [Project Juggler](https://eicweb.phy.anl.gov/EIC/juggler) to `./juggler`
    - [benchmarks/reconstruction_benchmarks](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks) to `./reconstruction_benchmarks`
  - suggestion: clone with SSH, especially if you will be contributing to
    them:
    ```
    git clone git@eicweb.phy.anl.gov:EIC/detectors/ip6.git
    git clone git@eicweb.phy.anl.gov:EIC/detectors/ecce.git
    git clone git@eicweb.phy.anl.gov:EIC/irt.git
    git clone git@eicweb.phy.anl.gov:EIC/eicd.git
    git clone git@eicweb.phy.anl.gov:EIC/juggler.git
    git clone git@eicweb.phy.anl.gov:EIC/benchmarks/reconstruction_benchmarks.git
    ```
  - follow directions below to build each module

## Environment
- execute `source environ.sh`
  - this file contains several environment variables needed by many scripts;
    it is recommended to read through `environ.sh` and make any changes as
    needed
  - `$BUILD_NPROC` is the number of parallel threads used for multi-threaded
    building and running Juggler multi-threaded
    - change it, if you prefer
    - memory-hungry builds will be built single-threaded
  - `$PRIMARY_PREFIX` is the main prefix where modules will be installed
    - by default, it should be `./opt/local`
    - change it, if you prefer
  - you can find documentation for many other variables in the corresponding
    module repositories
  - there are some additional "comfort" settings, which depend on your host
    environment; it is not required to use these, but feel free to add your own
    - if `~/bin` exists, it will be added to your `$PATH`
    - if you use Ruby shims via `rbenv`, it will make sure the container will
      use those, plus any corresponding gems
      - note: some miscellaneous scripts are in Ruby (extension `.rb`); if you
        want to run them, ask their developer for assistance

## Building Modules
- you must be in the EIC container (`opt/eic-shell`) and have environment
  variables set (`source environ.sh`)
- build each repository, one-by-one, in order of dependences (see
  [flowchart](doc/docDiagram.pdf) dependency graph)
  - build scripts, in recommended order:
  ```
  ./buildEICD.sh
  ./buildIRT.sh  # TODO: we need to update this for ECCE, you can ignore it for now...
  ./buildIP6.sh
  ./buildECCE.sh
  ./buildJuggler.sh # TODO: we need to also update this
  ```
  - you could also run `./rebuildAll.sh` to (re)build all of the modules in the
    recommended order
- instructions for the `reconstruction_benchmarks` repository are below

### Recommendations and Troubleshooting
- execute `./rebuildAll.sh` to quickly rebuild all repositories, in order of
  dependences; this is useful when you switch branches in *any* of the
  repositories, or if you pull in updates
  - sometimes things will break, simply because a dependent module is out of
    date; in that case, make sure all repositories are as up-to-date as
    possible; you may also need to update your Singularity/Docker image
    (`opt/update.sh`)
- be mindful of which branch you are on in each repository, especially if you
  have several active merge requests
  - for example, `irt` requires the new `eicd` components and datatypes, which
    at the time of writing this have not been merged to `eicd` `master`
  - use `./checkBranches.sh` to quickly check which branches you are on in all
    repositories
  - use `./checkStatus.sh` to run `git status` in each repository, which is
    useful during active development
- for clean builds, you can generally pass the word `clean` to any build script
  (you can also do `./rebuildAll.sh clean` to clean-build everything)
- most build scripts will run `cmake --build` multi-threaded
  - the `$BUILD_NPROC` environment variable should be set to the number of
    parellel threads you want to build with (see `environ.sh`)
  - careful, some module builds consume a lot of memory (Juggler); the build
    scripts will force single-threaded building for such cases

## Benchmarks Setup
The benchmarks run downstream of all other modules, and are useful for running
tests. For example, automated checks of upstream geometry changes, to see what
happens to performance plots. They are not required for upstream development,
but are certainly very useful. Currently we only have plots of raw hits; more
development is needed here.

Before running benchmarks, you must setup the common benchmarks:
```
pushd reconstruction_benchmarks
git clone git@eicweb.phy.anl.gov:EIC/benchmarks/common_bench.git setup
source setup/bin/env.sh
./setup/bin/install_common.sh
popd
source environ.sh
```
- these directions are similar to those in the
  [reconstruction_benchmarks Readme](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks/-/blob/master/README.md),
  but with some minor differences for our current setup (e.g., there is no need
  to build another detector in `reconstruction_benchmarks/.local`)
- the `source environ.sh` step will now set additional environment variables,
  since you now have common benchmarks installed
- there is no need to repeat this setup procedure, unless you want to start from
  a clean slate or update the common benchmarks

---

<a name="implementation"></a>
# Implementation

## Geometry and Materials
- the geometry and materials are implemented in DD4hep, in the
  [detectors/ecce](https://eicweb.phy.anl.gov/EIC/detectors/ecce) repository
  - see the [DD4hep class index](https://dd4hep.web.cern.ch/dd4hep/reference/)
    or the [homepage](https://dd4hep.web.cern.ch/dd4hep/) for documentation
  - the following files in `ecce/` are relevant for the dRICH:
    - `compact/drich.xml`: the compact file for the dRICH
      - constants for the geometry (e.g., dimensions, positions)
      - see `compact/definitions.xml` for main constants (for the full detector),
        such as global positioning
      - use `./searchCompactParams.sh [PATTERN]` to quickly obtain the
        *numerical* value of any constant, where `[PATTERN]` is case sensitive
        (e.g., `./searchCompactParams.sh DRICH`); this is a script in
        `drich-dev` which wraps `npdet_info`
      - see `comment` tags for details of all parameters
    - `compact/optical_materials.xml` for surface and material property tables,
      such as refractive index
      - see `compact/materials.xml` for material definitions and
        `compact/elements.xml` for elements
      - materials and parameterizations relevent for the dRICH contain the
        substring `DRICH` in their name
      - materials etc. are referenced by name in `compact/drich.xml`
      - most of these tables were obtained from the
        [common optics class](https://github.com/cisbani/dRICh/blob/main/share/source/g4dRIChOptics.hh)
    - the full detector compact file is `ecce.xml`, which is generated via
      Jinja during `cmake` (run `buildECCE.sh`), along with a dRICH-only
      compact file `ecce_drich_only.xml`; these compact files are used by many
      scripts, such as `npsim` (whereas `compact/drich.xml` is *only* for the
      dRICH implementation itself)
    - `src/DRICH_geo.cpp` is the C++ source file for the dRICH
      - relies on constants from the compact files
      - builds the dRICH
      - placement algorithms
      - parameterizations (e.g., of the mirrors)
      - see comments within the code for documentation

---

<a name="execution"></a>
# Execution

## Geometry
- run `./runDDwebDisplay.sh` to produce the geometry ROOT file
  - by default, it will use the compact file for the *full* detector
  - run `./runDDwebDisplay.sh d` to run on dRICH only
  - run `./runDDwebDisplay.sh p` to run on pfRICH only
  - output ROOT file will be in `geo/`
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
  - right click on the desired component, then click `Draw`
  - the default projection is perspective, but if you need to check alignment,
    change to orthographic projection:
    - right click -> show controls -> advanced -> orthographic camera
    - square your browser window aspect ratio, since the default aspect ratio is
      whatever your browser window is
  - more documentation found on [jsroot website](https://root.cern/js/)
- check for overlaps
  - typically more efficient to let the CI do this (in `detectors/ecce`)
  - call `./overlapCheck.sh` to run a local check
    - one check faster and less accurate, the other is slower and more accurate
- use `./searchCompactParams.sh [PATTERN]` to quickly obtain the value of any
  parameter in the compact files, rather than trying to "reverse" the formulas
  - for example, `./searchCompactParams.sh DRICH` to get all dRICH variables
  - the search pattern is case sensitive
  - this script is just a wrapper for `npdet_info`, run `npdet_info -h` for
    further guidance

### GDML Output
- currently we use the CI for this, from the `ecce` repository
  (the `athena` repository has a dRICH specific GDML output CI job, but at the
  time of writing this, this automation is not yet present in `ecce` CI)
- TODO: add a local script to automate connection to Fun4all

## Simulation
There are some local scripts to aid in simulation development; some of them have
been copied to the `reconstruction_benchmarks` repository, and may be more
up-to-date there.

All `.cpp` programs are compiled by running `make`, to corresponding `.exe`
executables.

- `simulate.py`: runs `npsim` with settings for the dRICH and pfRICH
  - run with no arguments for usage guidance
  - `npsim` is the main script for running Geant4 simulations with DD4hep
  - basically copied to `reconstruction_benchmarks`, but stored here as well for
    backup
- `drawHits.cpp`
  - reads simulation output and draws raw hit positions and number of hits vs.
    momentum
  - build with `make`, execute as `./drawHits.exe [simulation_output_file]`
  - specific for dRICH; for pfRICH version, see `pfrich/`
- `drawSegmentation.cpp`
  - reads simulation output and draws the hits within sensor pixels, which is
    useful for checking mapping of sensor segmentation (pixels)
  - relies on `text/sensorLUT.dat`, which must be up-to-date
    - you can produce a new version of this file by uncommenting relevant lines
      in `ecce/src/DRICH_geo.cpp` (search for `generate LUT`), and running
      something like `./rebuildAll.sh && ./runDDwebDisplay.sh`
  - build with `make`, execute with `./drawSegmentation.exe [simulation_output_file]`
  - specific for dRICH; for pfRICH version, see `pfrich/`

## Benchmarks
- use `./runBenchmark.sh` to run the simulation and subsequent reconstruction
  benchmarks
  - this is a wrapper for `reconstruction_benchmarks/benchmarks/rich/run_irt.sh`, 
    which is executed by the benchmarks CI
    - this script runs `npsim` and `juggler`
  - see also `reconstruction_benchmarks/benchmarks/rich/config.yml` for the
    commands used by the CI
  - it is practical to edit this wrapper script during development, for testing
    purposes; this is why several lines are commented out

## Miscellaneous
- the `math/` directory contains scripts and Mathematica notebooks used to
  perform miscellaneous calculations; many are "once and done" and don't really
  need to be implemented in the source code
- the `scripts/` directory contains all other miscellaneous scripts
- `deprecated/` contains some old scripts which may also be helpful


---


<a name="algorithms"></a>
# Algorithms

## IRT: Indirect Ray Tracing

- `createIRTauxfile.sh`: create IRT geometry auxiliary config file
