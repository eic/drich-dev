# irt-juggler-dev

## dependencies
- symlink or clone IRT repo to `./irt`
- symlink or clone Juggler repo to `./juggler`
- symlink simulation output directory to `./sim`
- if you want to use your own `athena` build, symlink or clone
  `detectors/athena` DD4hep repo to `./athena`; otherwise follow benchmarks setup
  directions below, which will install `athena` to a local prefix
- edit `environ.sh` if you want; this is needed by many scripts


## build irt
- follow `irt/README.md` (TL;DR: `irt/bin/buildIRT.sh`)


## build juggler
```
eic-shell
source environ.sh
./buildJuggler.sh
```


## benchmarks setup
```
git clone git@eicweb.phy.anl.gov:EIC/benchmarks/reconstruction_benchmarks.git benchmarks
pushd !$
git clone git@eicweb.phy.anl.gov:EIC/benchmarks/common_bench.git setup
source setup/bin/env.sh && ./setup/bin/install_common.sh
source .local/bin/env.sh && build_detector.sh
mkdir_local_data_link sim_output
mkdir -p results config
```
- cf. their associated readmes


## execution
```
eicshell
source environ.sh
./runJuggler.sh
```
