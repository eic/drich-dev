# dRICH-dev
Resources and Tools for EIC dRICH development 

| **Table of Contents**             |                                         |
| --:                               | ---                                     |
| [Setup](#setup)                   | How to download and build the code      |
| [Implementation](#implementation) | Where to find the code and what it does |
| [Execution](#execution)           | How to run the code                     |

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
  - execute `./eic-shell` to start the container; practically everything below
    must be executed within this container
- obtain EIC Software modules, either clone or symlink the repositories to the
  specified paths:
  - modules:
    - [ip6](https://github.com/eic/ip6) to `./ip6`
    - [epic](https://github.com/eic/epic) to `./epic`
    - [irt](https://eicweb.phy.anl.gov/EIC/irt) to `./irt`
    - [eicd](https://eicweb.phy.anl.gov/EIC/eicd) to `./eicd`
  - suggestion: clone with SSH, especially if you will be contributing to
    them:
    ```bash
    git clone git@github.com:eic/epic.git
    git clone git@github.com:eic/ip6.git
    git clone git@eicweb.phy.anl.gov:EIC/irt.git
    git clone git@eicweb.phy.anl.gov:EIC/eicd.git
    ```
  - follow directions below to build each module

## Environment
- execute `source environ.sh`
  - this file contains several environment variables needed by many scripts;
    it is recommended to read through `environ.sh` and make any changes as
    needed
  - `$BUILD_NPROC` is the number of parallel threads used for multi-threaded
    building and running multi-threaded
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

## Building Modules
- you must be in the EIC container (`eic-shell`) and have environment
  variables set (`source environ.sh`)
- build each repository, one-by-one, in order of dependences (see
  [flowchart](doc/docDiagram.pdf) dependency graph)
  - build scripts, in recommended order:
  ```bash
  ./build_eicd.sh
  ./build_irt.sh  # TODO: we need to update this for EPIC, you can ignore it for now...
  ./build_ip6.sh
  ./build_epic.sh
  ```
  - you could also run `./rebuild_all.sh` to (re)build all of the modules in the
    recommended order

### Recommendations and Troubleshooting
- execute `./rebuild_all.sh` to quickly rebuild all repositories, in order of
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
  - use `./check_branches.sh` to quickly check which branches you are on in all
    repositories
  - use `./check_status.sh` to run `git status` in each repository, which is
    useful during active development
- for clean builds, you can generally pass the word `clean` to any build script
  (you can also do `./rebuild_all.sh clean` to clean-build everything)
- most build scripts will run `cmake --build` multi-threaded
  - the `$BUILD_NPROC` environment variable should be set to the number of
    parellel threads you want to build with (see `environ.sh`)

## Benchmarks Setup
- TODO: in light of the reconstruction framework change, the benchmarks will need
  to be updated; any local benchmark code will be updated or deprecated, but
  we leave the current documentation here for reference:

The benchmarks run downstream of all other modules, and are useful for running
tests. For example, automated checks of upstream geometry changes, to see what
happens to performance plots. They are not required for upstream development,
but are certainly very useful. Currently we only have plots of raw hits; more
development is needed here.

Before running benchmarks, you must setup the common benchmarks:
```bash
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

Now install the [reconstruction benchmarks](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks)
```bash
git clone git@eicweb.phy.anl.gov:EIC/benchmarks/reconstruction_benchmarks.git
```


---

<a name="implementation"></a>
# Implementation

## Geometry and Materials
- the geometry and materials are implemented in DD4hep, in the
  [epic](https://github.com/eic/epic) repository
  - see the [DD4hep class index](https://dd4hep.web.cern.ch/dd4hep/reference/)
    or the [homepage](https://dd4hep.web.cern.ch/dd4hep/) for documentation
  - the following files in `epic/` are relevant for the dRICH:
    - `compact/drich.xml`: the compact file for the dRICH
      - constants for the geometry (e.g., dimensions, positions)
      - see `compact/definitions.xml` for main constants (for the full detector),
        such as global positioning
      - use `./search_compact_params.sh [PATTERN]` to quickly obtain the
        *numerical* value of any constant, where `[PATTERN]` is case sensitive
        (e.g., `./search_compact_params.sh DRICH`); this is a script in
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
    - the full detector compact file is `epic.xml`, which is generated via
      Jinja during `cmake` (run `build_epic.sh`), along with a dRICH-only
      compact file `epic_drich_only.xml`; these compact files are used by many
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
- run `./run_dd_web_display.sh` to produce the geometry ROOT file
  - by default, it will use the compact file for the *full* detector
  - run `./run_dd_web_display.sh d` to run on dRICH only
  - output ROOT file will be in `geo/`
- open the resulting ROOT file in `jsroot` geoviewer, using either:
  - [CERN host](https://root.cern/js/) (recommended)
  - Local host (advanced, but offers better control) - see [setup guide](doc/jsroot.md)
  - [ANL hosted](https://eic.phy.anl.gov/geoviewer/)
- browse the ROOT file geometry tree in the sidebar on the left:
  ```
  detector_geometry.root
  └── default
      └── world_volume
          ├── ...
          ├── DRICH
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
  - typically more efficient to let the CI do this (in `epic`)
  - call `./overlap_check.sh` to run a local check
    - one check faster and less accurate, the other is slower and more accurate
- use `./search_compact_params.sh [PATTERN]` to quickly obtain the value of any
  parameter in the compact files, rather than trying to "reverse" the formulas
  - for example, `./search_compact_params.sh DRICH` to get all dRICH variables
  - the search pattern is case sensitive
  - this script is just a wrapper for `npdet_info`, run `npdet_info -h` for
    further guidance


### GDML Output
- currently we use the CI for this, from the `epic` repository
  (the `athena` repository has a dRICH specific GDML output CI job, but at the
  time of writing this, this automation is not yet present in `epic` CI)
- TODO: add a local script to automate connection to Fun4all

## Simulation
There are some local scripts to aid in simulation development; some of them have
been copied to the `reconstruction_benchmarks` repository, and may be more
up-to-date there.

All `src/.cpp` programs are compiled by running `make`, which will build corresponding
executables and install them to `bin/`

- `simulate.py`: runs `npsim` with settings for the dRICH and pfRICH
  - run with no arguments for usage guidance
  - `npsim` is the main script for running Geant4 simulations with DD4hep
  - basically copied to `reconstruction_benchmarks`, but stored here as well for
    backup
- example simulation analysis code is found in `src/examples`
  - see comments within each for more details
  - build with `make` (from the top-level directory); the corresponding executables
    will be installed to `bin/`
- `src/draw_hits.cpp` (run with `bin/draw_hits`)
  - reads simulation output and draws raw hit positions and number of hits vs.
    momentum
  - build with `make`, execute as `bin/draw_hits [simulation_output_file]`
  - specific for dRICH; for pfRICH version, see `deprecated/pfrich/`
- `src/draw_segmentation.cpp` (run with `bin/draw_segmentation`)
  - reads simulation output and draws the hits within sensor pixels, which is
    useful for checking mapping of sensor segmentation (pixels)
  - relies on `text/sensorLUT.dat`, which must be up-to-date
    - you can produce a new version of this file by uncommenting relevant lines
      in `epic/src/DRICH_geo.cpp` (search for `generate LUT`), and running
      something like `./rebuild_all.sh && ./run_dd_web_display.sh`
  - build with `make`, execute with `bin/draw_segmentation [simulation_output_file]`
  - specific for dRICH; for pfRICH version, see `deprecated/pfrich/`

## Benchmarks
- TODO: in light of the reconstruction framework change, the benchmarks will need
  to be updated; any local benchmark code will be updated or deprecated, but
  we leave the current documentation here for reference:
  - use `./run_benchmark.sh` to run the simulation and subsequent reconstruction
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
- the `scripts/` directory contains all other miscellaneous scripts; 
  - some scripts are in Ruby; follow [this guide](doc/ruby.md) to install gems
  (dependencies)
- `deprecated/` contains some old scripts which may also be helpful
