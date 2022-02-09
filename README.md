# drich-dev
Scripts for ATHENA dRICH and IRT development 

ATHENA Software is modular: see [the flowchart overview](docDiagram.pdf) for general guidance of the modules and their connections of calls and data flow


## dependencies
- install EIC Software container:
  - `opt/update.sh` will install or update the EIC Software container automatically
    - the `opt` directory will contain your prefix, `ATHENA_PREFIX=opt/local`
    - execute `opt/eic-shell` will start the container
    - execute `echo $ATHENA_PREFIX`
      - it should be `./opt/local` unless you changed it
      - this prefix will be used for builds of ATHENA Software modules
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

## environment
- edit `environ.sh` if you want; this contains several env vars needed by many scripts
  - you can find documentation for many variables in the corresponding repositories

## build modules
### athena and ip6



## build irt
- follow `irt/README.md` (TL;DR: `irt/bin/buildIRT.sh`)


## build detectors (optional)
- `./buildATHENA.sh`
- `./buildIP6.sh`
- check geometry with `./runDDwebDisplay.sh` and open `$(ls -t *.root|head -n1)` in `jsroot`


## build EICD
- `./buildEICD.sh`


## benchmarks setup
```
git clone git@eicweb.phy.anl.gov:EIC/benchmarks/reconstruction_benchmarks.git benchmarks
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


## build juggler
```
source environ.sh
./buildJuggler.sh
```
- note: `buildJuggler.sh` will run `kluge.sh` to correct an issue with
  component finding (see comments in the script)


## execution
```
source environ.sh
./runJuggler.sh  #OR#  ./runBenchmark.sh
# use ./rebuildAll.sh to quickly build all repos
```
