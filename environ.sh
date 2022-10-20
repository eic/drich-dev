#!/bin/bash

# drich-dev path
if [ -z "${BASH_SOURCE[0]}" ]; then
  export DRICH_DEV=$(dirname $(realpath $0))
else
  export DRICH_DEV=$(dirname $(realpath ${BASH_SOURCE[0]}))
fi

# obtain number of CPUs
# - set it manually, if you prefer, or if auto detection fails
export BUILD_NPROC=$([ $(uname) = 'Darwin' ] && sysctl -n hw.ncpu || nproc)
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
echo "detected $BUILD_NPROC cpus"

# cmake packages
export IRT_ROOT=$EIC_SHELL_PREFIX  # overrides container version with local version
export EDM4EIC_ROOT=$EIC_SHELL_PREFIX  # overrides container version with local version

# # source environment from reconstruction_benchmarks
# if [ -f "reconstruction_benchmarks/.local/bin/env.sh" ]; then
#   pushd reconstruction_benchmarks
#   source .local/bin/env.sh
#   popd
# fi

export LOCAL_DATA_PATH=$DRICH_DEV

# source common upstream environment (nightly jug_xl build)
source /opt/detector/setup.sh

# source local environment (a build target from `epic`)
# - overrides upstream `$DETECTOR*` vars
# - prioritizes `$EIC_SHELL_PREFIX/lib` in `$LD_LIBRARY_PATH`
[ -f $EIC_SHELL_PREFIX/setup.sh ] && source $EIC_SHELL_PREFIX/setup.sh

# source EICrecon installation
#
# FIXME: update prefix
#
#
[ -f $DRICH_DEV/EICrecon/install/bin/eicrecon-this.sh ] && source $DRICH_DEV/EICrecon/install/bin/eicrecon-this.sh

# environment overrides:
# - prefer local juggler build
export JUGGLER_INSTALL_PREFIX=$EIC_SHELL_PREFIX
export JUGGLER_N_THREADS=$BUILD_NPROC
# - update prompt
export PS1="${PS1:-}"
export PS1="drich${PS1_SIGIL}>${PS1#*>}"
unset branch

# prioritize local build targets
export LD_LIBRARY_PATH=$DRICH_DEV/lib:$EIC_SHELL_PREFIX/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$EIC_SHELL_PREFIX/python:$PYTHONPATH

# use local rbenv ruby shims, if installed
export RBENV_ROOT=$DRICH_DEV/.rbenv
if [ -d "$RBENV_ROOT" ]; then
  export PATH=$RBENV_ROOT/bin:$PATH
  eval "$(.rbenv/bin/rbenv init - bash)"
  export PYTHON=$(which python) # for pycall gem
fi

# additional comfort settings; add your own here
# - PATH additions
export PATH=.:$PATH                                   # ./
export PATH=$DRICH_DEV/bin:$PATH                      # drich-dev/bin
[ -d "${HOME}/bin" ] && export PATH=$PATH:${HOME}/bin # ~/bin
# - shell settings and aliases
shopt -s autocd # enable autocd (`alias <dirname>='cd <dirname>'`)
alias ll='ls -lhp --color=auto'

# print environment
echo """


     ###########################################
     ###    dRICH Development Environment    ###
     ###########################################

Beam:
  BEAMLINE_PATH           = $BEAMLINE_PATH
  BEAMLINE_CONFIG         = $BEAMLINE_CONFIG
  BEAMLINE_CONFIG_VERSION = $BEAMLINE_CONFIG_VERSION

Detector:
  DETECTOR         = $DETECTOR
  DETECTOR_PATH    = $DETECTOR_PATH
  DETECTOR_CONFIG  = $DETECTOR_CONFIG
  DETECTOR_VERSION = $DETECTOR_VERSION

Packages:
  IRT_ROOT = $IRT_ROOT
  EDM4EIC  = $EDM4EIC

Juggler (to be deprecated):
  JUGGLER_INSTALL_PREFIX   = $JUGGLER_INSTALL_PREFIX
  JUGGLER_DETECTOR         = $JUGGLER_DETECTOR
  JUGGLER_DETECTOR_PATH    = $JUGGLER_DETECTOR_PATH
  JUGGLER_DETECTOR_CONFIG  = $JUGGLER_DETECTOR_CONFIG
  JUGGLER_DETECTOR_VERSION = $JUGGLER_DETECTOR_VERSION
  JUGGLER_DETECTOR_PATH    = $JUGGLER_DETECTOR_PATH
  JUGGLER_BEAMLINE_CONFIG  = $JUGGLER_BEAMLINE_CONFIG
  JUGGLER_BEAMLINE_CONFIG_VERSION = $JUGGLER_BEAMLINE_CONFIG_VERSION

LD_LIBRARY_PATH:
  $(echo $LD_LIBRARY_PATH | sed 's/:/\n  /g')

Common:
  DRICH_DEV        = $DRICH_DEV
  BUILD_NPROC      = $BUILD_NPROC
  EIC_SHELL_PREFIX = $EIC_SHELL_PREFIX
  DETECTOR_PATH    = $DETECTOR_PATH

"""
