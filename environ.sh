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
export LOCAL_DATA_PATH=$(pwd)

# source common environment, then override some settings
source /opt/detector/setup.sh
# prefer local compact files
export DETECTOR_PATH=$(pwd)/epic
# prefer local juggler build
export JUGGLER_INSTALL_PREFIX=$PRIMARY_PREFIX
export JUGGLER_DETECTOR_PATH=$DETECTOR_PATH
export JUGGLER_N_THREADS=$BUILD_NPROC
# prioritize local build targets
export LD_LIBRARY_PATH=$PRIMARY_PREFIX/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PRIMARY_PREFIX/python:$PYTHONPATH

# print environment
if [ -f "reconstruction_benchmarks/.local/bin/env.sh" ]; then
  printf "\n\n--------------------------------\n"
  print_env.sh
  echo "--------------------------------"
fi

# use local rbenv ruby shims, if installed
export RBENV_ROOT=$(pwd)/.rbenv
if [ -d "$RBENV_ROOT" ]; then
  export PATH=$RBENV_ROOT/bin:$PATH
  eval "$(.rbenv/bin/rbenv init - bash)"
  export PYTHON=$(which python) # for pycall gem
fi

### additional comfort settings, some dependent on host machine; 
### feel free to add your own here
export PATH=$(pwd)/bin:$PATH  # add ./bin to $PATH
export PATH=.:$PATH  # add ./ to $PATH
shopt -s autocd      # enable autocd
if [ -d "${HOME}/bin" ]; then export PATH=$PATH:${HOME}/bin; fi   # add ~/bin to $PATH
