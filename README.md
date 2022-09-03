# dRICH-dev
Resources and Tools for EPIC dRICH development 

| **Table of Contents**               |                                                       |
| --:                                 | ---                                                   |
| [Setup](#setup)                     | How to download and build the code                    |
| [Geometry and Materials](#geometry) | Detector geometry and material properties description |
| [Simulation](#simulation)           | Running the simulation in Geant4                      |
| [Reconstruction](#reconstruction)   | Running the reconstruction algorithms                 |
| [Miscellaneous](#miscellaneous)     | Additional code for support                           |

| **Documentation Links**                       |                                               |
| --:                                           | ---                                           |
| [Flowchart](doc/docDiagram.pdf)               | Diagram of software modules                   |
| [Links](doc/links.md)                         | Collection of dRICH Software and Resources    |
| [Branches and Pull Requests](doc/branches.md) | Active development branches and pull requests |
| [Project Board](https://github.com/orgs/eic/projects/4) | Issues tracking                     |

## Notes
EPIC Software is modular: see [the flowchart overview](doc/docDiagram.pdf) for
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
  has since been modified to support EPIC
- See [doc/athena.md](doc/athena.md) for guidance how to run using the ATHENA geometry;
  this is temporarily supported for helping test reconstruction algorithms
- See [doc/athena-branches.md](doc/athena-branches.md) for information about the
  development branches and pull requests that were used for the ATHENA proposal
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
  - follow [eic-container documentation](https://eicweb.phy.anl.gov/containers/eic_container)
    to obtain the EIC software image
    - the `eic-shell` script is used to start a container shell
    - all documentation below assumes you are running in `eic-shell`
    - this image contains all the dependencies needed for EPIC simulations
      - tip: when in a container shell (`eic-shell`), see `/opt/software/linux.../gcc.../`
        for the installed software
        - for example, if you want to check exactly what is available in the
          [EDM4hep data model](https://github.com/key4hep/EDM4hep), see the headers
          in `/opt/software/linux.../gcc.../edm4hep.../include/edm4hep/` (these are
          produced by the [edm4hep.yaml](https://github.com/key4hep/EDM4hep/blob/master/edm4hep.yaml)
          configuration file)
    - be sure to regularly update your image by running `eic-shell --upgrade`; this is necessary
      to keep up with upstream changes, such as in EDM4hep or DD4hep
  - alternatively, we have a wrapper script:
    - Run `opt/update.sh` to obtain (or update) the EIC Software image automatically
    - the image and builds will be stored in `./opt`
    - run `opt/eic-shell` to start a container
    - this wrapper script may not be supported in the future (use `eic-shell --upgrade` instead)
- Obtain EPIC Software modules, either clone or symlink the repositories to the
  specified paths:
  - Modules:
    - [epic](https://github.com/eic/epic) to `./epic`, for the EPIC detector geometry,
      based on [DD4hep](https://github.com/AIDASoft/DD4hep)
    - [irt](https://github.com/eic/irt) to `./irt`, for the Indirect Ray Tracing for RICH reconstruction
    - [EDM4eic](https://github.com/eic/EDM4eic) to `./EDM4eic`, for the data model; this extends
      [EDM4hep](https://github.com/key4hep/EDM4hep), the common data model, which is included
      in the EIC software image
    - [juggler](https://eicweb.phy.anl.gov/EIC/juggler) to `./juggler`, for the reconstruction
      framework used in ATHENA, and supported while we migrate to the new reconstruction framework in EPIC
  - Suggestion: clone with SSH, which is required for contributions:
    ```bash
    git clone git@github.com:eic/epic.git
    git clone git@github.com:eic/irt.git
    git clone git@github.com:eic/EDM4eic.git
    git clone git@eicweb.phy.anl.gov:EIC/juggler.git
    ```
  - Checkout the appropriate branches of each repository, depending on your needs
    - see [Branches and Pull Requests](doc/branches.md)
    - for example, currently the IRT code runs in `juggler` and relies on a
      custom data model in `EDM4eic`, neither of which have been merged to the
      main branches; the "IRT Development" branches are recommended for running
      the IRT code for now, until IRT is integrated with the new reconstruction
      framework
    - see also the [project page](https://github.com/orgs/eic/projects/4/views/1)
      for more up-to-date information
  - Follow directions below to build each module

## Environment
- execute `source environ.sh`
  - this file contains several environment variables needed by many scripts;
    it is recommended to read through `environ.sh` and make any changes as
    needed
  - `$BUILD_NPROC` is the number of parallel threads used for multi-threaded
    building and running multi-threaded
    - change it, if you prefer
    - memory-hungry builds will be built single-threaded
  - `$EIC_SHELL_PREFIX` is the main directory where module builds will be installed
    - by default, it is `<path to eic-shell>/local`
    - change it, if you prefer a different directory
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
  ./build_EDM4eic.sh
  ./build_irt.sh
  ./build_epic.sh
  ./build_juggler.sh
  ```
  - you could also run `./rebuild_all.sh` to (re)build all of the modules in the
    recommended order
- run `source environ.sh` again, if:
  - if this is your first time building, or a clean build
  - if a module's environment has been updated, in particular `epic/templates/setup.sh.in`

### Recommendations and Troubleshooting
- be mindful of the environment variables
  - if in doubt, run `source environ.sh` to update all of them
  - inspect all of the printed environment variables
- execute `./rebuild_all.sh` to quickly rebuild all repositories, in order of
  dependences; this is useful when you switch branches in *any* of the
  repositories, or if you pull in updates
  - sometimes things will break, simply because a dependent module is out of
    date; in that case, make sure all repositories are as up-to-date as
    possible; you may also need to update your Singularity/Docker image
    (`eic-shell --upgrade`)
- be mindful of which branch you are on in each repository, especially if you
  have several active pull requests
  - for example, `irt` requires the new `EDM4eic` components and datatypes, which
    at the time of writing this have not been merged to `EDM4eic` `master`
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

<a name="geometry"></a>
# Geometry and Materials
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
    - the full detector compact file is `$DETECTOR_PATH/epic.xml`, which is generated via
      Jinja during `cmake` (run `build_epic.sh`), along with a dRICH-only
      compact file `$DETECTOR_PATH/epic_drich_only.xml`
      - these compact files are used by many scripts, such as `npsim`, whereas
        `compact/drich.xml` is *only* for the dRICH implementation itself
      - `build_epic.sh` (`cmake`) will also copy local `epic/compact/*.xml`
        files to `$DETECTOR_PATH`, since the generated compact files (`$DETECTOR_PATH/epic*.xml`) 
        reference compact files in `$DETECTOR_PATH`
    - `src/DRICH_geo.cpp` is the C++ source file for the dRICH
      - relies on constants from the compact files
      - builds the dRICH
      - placement algorithms
      - parameterizations (e.g., of the mirrors)
      - see comments within the code for documentation

## Viewing the Geometry and Parameter Values
- run `./run_dd_web_display.sh` to produce the `TGeo` geometry ROOT file
  - follow the usage guide to specify whether to draw the full EPIC
    detector, or just the dRICH
  - output ROOT file will be in `geo/`, by default
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

## GDML Output
- currently we use the CI for this, from the `epic` repository
  (the `athena` repository has a dRICH specific GDML output CI job, but at the
  time of writing this, this automation is not yet present in `epic` CI)
- TODO: add a local script to automate connection to Fun4all

---

<a name="simulation"></a>
# Simulation
There are some local scripts to aid in simulation development. All compilable
`src/.cpp` programs are compiled by running `make`, which will build
corresponding executables and install them to `bin/`

- `simulate.py`: runs `npsim` with settings for the dRICH and pfRICH
  - run with no arguments for usage guidance
  - `npsim` is the main script for running Geant4 simulations with DD4hep; it
    wraps DD4hep's `ddsim` with some extra settings for Cherenkov detectors,
    such as the sensitive detector action
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

## Automated Parameter Variation
- use `scripts/vary_params.rb` to run simulation jobs while varying dRICH compact file parameters
  - Ruby gems (dependencies) are required to run this; see [doc/ruby.md](doc/ruby.md) for guidance
  - The input of this script is a configuration file, written as a class
    - This file includes:
      - Which parameters to vary, and how
      - Pipelines: shell commands to run on each variant, for example, `simulate.py`
    - See `ruby/variator/template.rb` for an example and more details
      - The class `Variator` inherits from the common superclass `VariatorBase`
      - Add your own `Variator` class in another file, then specify this file
        when you call `vary_params.rb`, so that it will use your `Variator` class
        rather than the default
    - The script runs multi-threaded: one thread per variant
      - Output `stdout` and `stderr` are logged, along with your shell command pipelines

---

<a name="reconstruction"></a>
# Reconstruction

## IRT: Indirect Ray Tracing

We currently use `irt` both as a standalone reconstruction algorithm and integrated in `juggler`
as `IRTAlgorithm`. The `juggler` implementation was used for ATHENA, and is supported for EPIC
until it is time to migrate to the new reconstruction framework.

Procedure:

- Create the auxiliary IRT configuration file; this uses a temporary "backdoor"
  dependency on `irt` in `epic` to produce a ROOT file containing `libIRT`
  objects, such as optical boundaries, based on the dRICH geometry description.
```bash
scripts/create_irt_auxfile.sh
```

- Run the simulation, for example:
```bash
simulate.py -t 1 -s -n 50
```

- Run the reconstruction via Juggler, or try the stand-alone reader macro:
```bash
recon.sh -j   # to use Juggler (IRTAlgorithm)
recon.sh -r   # to use standalone reader (irt/scripts/reader*.C)
recon.sh -h   # for usage guide, such as how to specify input/output files
```

- Run the evaluation code (use `-h` for usage):
```bash
evaluate.sh
```

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


---

<a name="miscellaneous"></a>
# Miscellaneous
- the `math/` directory contains scripts and Mathematica notebooks used to
  perform miscellaneous calculations; many are "once and done" and don't really
  need to be implemented in the source code
- the `scripts/` directory contains all other miscellaneous scripts; 
  - some scripts are in Ruby; follow [this guide](doc/ruby.md) to install gems
  (dependencies)
- `deprecated/` contains some old scripts which may also be helpful
