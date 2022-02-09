# irt-juggler-dev

## dependencies
- clone or symlink [athena](https://eicweb.phy.anl.gov/EIC/detectors/athena) to `./athena`
- clone or symlink [ip6](https://eicweb.phy.anl.gov/EIC/detectors/ip6) to `./ip6`
- clone or symlink [IRT](https://eicweb.phy.anl.gov/EIC/irt) to `./irt`
- clone or symlink [Juggler](https://eicweb.phy.anl.gov/EIC/juggler) to `./juggler`
- clone or symlink [EICD](https://eicweb.phy.anl.gov/EIC/eicd) to `./eicd`
- install EIC Software container
  - `opt/update.sh` will install or update the EIC Software container automatically
  - the `opt` directory will contain your prefix, `ATHENA_PREFIX=opt/local`
  - if you prefer, follow the [eic_container documentation](https://eicweb.phy.anl.gov/containers/eic_container)

## environment
- edit `environ.sh` if you want; this contains several env vars needed by many scripts
  - you can find documentation for many variables in the corresponding repositories

**HERE**


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
