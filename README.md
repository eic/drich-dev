# irt-juggler-dev

## setup
- symlink Juggler repo to `juggler`
- symlink simulation output directory to `sim`
- symlink `detectors/athena` DD4hep repo to `athena`; you must have a build of this
- edit `environ.sh` if you want; this is sourced by many scripts


## build juggler
```
eic-shell
source environ.sh
buildJuggler.sh
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
runJuggler.sh
```
