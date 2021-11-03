# irt-juggler-dev

## dependencies
- symlink or clone IRT repo to `./irt`; should be on `main` branch
- symlink or clone Juggler repo to `./juggler`; should be on `irt-init` branch (or your own)
- symlink or clone EICD repo to `./eicd`; should be on `ayk-00` branch
- symlink simulation output directory to `./sim`
- if you want to use your own `athena` build, symlink or clone
  `detectors/athena` DD4hep repo to `./athena` and `detectors/ip6` to `./ip6`;
  otherwise follow benchmarks setup directions below, which will install
  `athena` to a local prefix
- edit `environ.sh` if you want; this is needed by many scripts
- most commands below need to be executed in the EIC container (`eic-shell`)


## build irt
- follow `irt/README.md` (TL;DR: `irt/bin/buildIRT.sh`)


## build detectors (optional)
- `./buildATHENA.sh`
- `./buildIP6.sh`


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
