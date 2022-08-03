#!/bin/bash

# obtain number of CPUs
# - set it manually, if you prefer, or if auto detection fails
export BUILD_NPROC=$([ $(uname) = 'Darwin' ] && sysctl -n hw.ncpu || nproc)
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
echo "detected $BUILD_NPROC cpus"

# primary prefix: 
# note: if you prefer a different prefix, change it here
export PRIMARY_PREFIX=$EIC_SHELL_PREFIX

# cmake packages
export IRT_ROOT=$PRIMARY_PREFIX # overrides container version with local version
export EICD_ROOT=$PRIMARY_PREFIX # overrides container version with local version

# environment from reconstruction_benchmarks
if [ -f "reconstruction_benchmarks/.local/bin/env.sh" ]; then
  pushd reconstruction_benchmarks
  source .local/bin/env.sh
  popd
fi

# fix env vars which would have been overwritten by `reconstruction_benchmarks/.local/bin/env.sh`:
export DETECTOR_PATH=$(pwd)/epic
#export BEAMLINE_CONFIG_VERSION=master
#export DETECTOR_VERSION=master

if [ -f "reconstruction_benchmarks/.local/bin/env.sh" ]; then
  printf "\n\n--------------------------------\n"
  print_env.sh
  echo "--------------------------------"
fi

### additional comfort settings, some dependent on host machine; 
### feel free to add your own here
export PATH=$(pwd)/bin:$PATH  # add ./bin to $PATH
export PATH=.:$PATH  # add ./ to $PATH
shopt -s autocd      # enable autocd
if [ -d "${HOME}/bin" ]; then export PATH=${HOME}/bin:$PATH; fi   # add ~/bin to $PATH
if [ -n "$(which rbenv)" ]; then eval "$(rbenv init - bash)"; fi  # switch to host's rbenv ruby shim (and its gems)
