# paths
#export JUGGLER_INSTALL_PREFIX=$(pwd)/juggler/install
export JUGGLER_INSTALL_PREFIX=$ATHENA_PREFIX
export LD_LIBRARY_PATH=$JUGGLER_INSTALL_PREFIX/lib:$LD_LIBRARY_PATH

# cmake packages
export IRT_ROOT=$ATHENA_PREFIX # overrides container version with local version
export EICD_ROOT=$ATHENA_PREFIX # overrides container version with local version

# juggler config vars
export JUGGLER_DETECTOR="athena"
export BEAMLINE_CONFIG="ip6"
export JUGGLER_SIM_FILE=$(pwd)/sim/sim_run.root
export JUGGLER_REC_FILE=test.root
export JUGGLER_N_EVENTS=100
export JUGGLER_RNG_SEED=1
export JUGGLER_N_THREADS=6

# environment from reconstruction benchmarks
if [ -f "benchmarks/.local/bin/env.sh" ]; then
  pushd benchmarks
  source .local/bin/env.sh
  echo "--------------"
  print_env.sh
  popd
fi

# juggler config vars which would have been overwritten by 
# `benchmarks/.local/bin/env.sh`:
# - maybe these could be used to ensure we use preferred
#   detector build, rather than the one built with the
#   `benchmarks` repo
#export BEAMLINE_CONFIG_VERSION=master
#export JUGGLER_DETECTOR_VERSION=master
#export DETECTOR_PATH=$(pwd)/athena
#export DETECTOR_VERSION=master
