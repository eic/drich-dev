# dRICH-dev
Scripts for ATHENA dRICH and IRT development 

ATHENA Software is modular: see [the flowchart overview](docDiagram.pdf) for general guidance of the modules and their connections of calls and data flow

## Dependencies
- install EIC Software container:
  - `opt/update.sh` will install or update the EIC Software container automatically
    - execute `opt/eic-shell` to start the container
    - execute `echo $ATHENA_PREFIX`
      - it should be `./opt/local` unless you changed it
      - this is the prefix that will be used for builds of ATHENA Software modules (edit `environ.sh` to change prefixes)
  - alternatively, follow the [eic_container documentation](https://eicweb.phy.anl.gov/containers/eic_container)
    - `opt/update.sh` is a wrapper for this procedure for storing the image and prefix locally in `opt/` 
- obtain ATHENA Software modules, either clone or symlink the repositories to the specified paths
  - modules:
    - [ip6](https://eicweb.phy.anl.gov/EIC/detectors/ip6) to `./ip6`
    - [athena](https://eicweb.phy.anl.gov/EIC/detectors/athena) to `./athena`
    - [IRT](https://eicweb.phy.anl.gov/EIC/irt) to `./irt`
    - [EICD](https://eicweb.phy.anl.gov/EIC/eicd) to `./eicd`
    - [Juggler](https://eicweb.phy.anl.gov/EIC/juggler) to `./juggler`
    - [reconstruction benchmarks](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks) to `./reconstruction_benchmarks`
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
    - this will be the number of CPUs used for multi-threaded building
    - reduce it, if you prefer
    - memory-hungry builds will be built single-threaded
  - edit other variables if you want, e.g., for specifying alternate prefixes; you can find documentation for many variables in the corresponding module repositories

## Building Modules
- you must be in the EIC container (`opt/eic-shell`) and have environment variables set (`source environ.sh`)
- build each repository, one-by-one, in order of dependences (see [flowchart](docDiagram.pdf) dependency graph)
- instructions for the `reconstructions_benchmarks` repository are below
```
./buildEICD.sh
./buildIRT.sh
./buildIP6.sh
./buildATHENA.sh
./buildJuggler.sh
```

### Recommendations and Troubleshooting
- execute `./rebuildAll.sh` to quickly rebuild all repositories, in order of dependences; this is useful when you switch branches in any of they repositories
- be mindful of which branch you are on in each repository, especially if you have several active merge requests
  - for example, `IRT` requires the new `eicd` components and datatypes, which at the time of writing this have not been merged to `eicd` `master`
- for clean builds, you can generally pass the word `clean` to any build script (you can also do `./rebuildAll.sh clean` to clean build everything)
- most build scripts will run `cmake --build` multi-threaded
  - the `$BUILD_NCPUS` environment variable should be set to the number of CPUs you want to build with (see `environ.sh`)
  - careful, some module builds consume a lot of memory (Juggler); the build scripts will force single-threaded building for such cases

### Benchmarks Setup
TODO: UPDATE THIS
```
pushd !$
git clone git@eicweb.phy.anl.gov:EIC/benchmarks/common_bench.git setup
source setup/bin/env.sh && ./setup/bin/install_common.sh
source .local/bin/env.sh
build_detector.sh # if you want to
mkdir_local_data_link sim_output
mkdir -p results config
```
- checkout the correct development branch (currently `irt-benchmark`) of
  `reconstruction_benchmarks`
- cf. their readmes for further documentation
- files will be installed in a `.local` directory; you may not want to install a build of `athena` 
  if you are using your own build somewhere else

## Execution

## view athena detector
- check geometry with `./runDDwebDisplay.sh` and open `$(ls -t *.root|head -n1)` in `jsroot`





## execution
```
source environ.sh
./runJuggler.sh  #OR#  ./runBenchmark.sh
# use ./rebuildAll.sh to quickly build all repos
```
