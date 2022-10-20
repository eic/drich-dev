#!/bin/bash
# general cmake wrapper, with module-specific settings

set -e

### custom compiler ##################
# export CC=gcc; export CXX=g++;
# export CC=clang; export CXX=clang++;
######################################

# check environment
if [ -z "$DRICH_DEV" ]; then echo "ERROR: source environ.sh"; exit 1; fi
if [ -z "$BUILD_NPROC" ]; then export BUILD_NPROC=1; fi

# set module, and check for existence
if [ $# -eq 0 ]; then
  echo """
  USAGE: $0 [MODULE] [OPTIONS]

  - [MODULE] is a repository directory, e.g., \"epic\"

  - [OPTIONS] will be passed to cmake
    - pass no OPTIONS to use the defaults set in $0
    - if the first OPTION is:
      - \"clean\": the build and install directories will be cleaned
      - \"fast\": skip buildsystem generation
  """
  exit 2
fi
module=$(echo $1 | sed 's;/$;;')
shift
if [ ! -d "$module" ]; then
  echo "ERROR: module \"$module\" does not exist"
  exit 1
fi

# determine if clean build, and set extraOpts
clean=0
fast=0
extraOpts=""
if [ $# -ge 1 ]; then
  case $1 in
    clean) clean=1; shift; ;;
    fast)  fast=1;  shift; ;;
  esac
  extraOpts=$*
fi
echo """
BUILDING:
module    = $module
clean     = $clean
fast      = $fast 
extraOpts = $extraOpts
"""

########################################
# common options
prefix=$EIC_SHELL_PREFIX
nproc=$BUILD_NPROC

########################################
# module-specific options and preparation
genOpts=""
function genOpt() { genOpts+="-D$* "; }
case $module in
  EDM4eic)
    genOpt BUILD_DATA_MODEL=ON
    ;;
  irt)
    genOpt DELPHES=ON
    genOpt EVALUATION=OFF
    ;;
  epic)
    ;;
  EICrecon)
    genOpts+="-LAH " # dump variables
    prefix=$module/install # FIXME: delete this line when ready
    genOpt CMAKE_FIND_DEBUG_MODE=OFF
    ;;
  juggler)
    prefix=$JUGGLER_INSTALL_PREFIX
    nproc=2 # maybe memory hungry
    genOpt CMAKE_FIND_DEBUG_MODE=OFF
    ;;
  athena)
    genOpt IRT_AUXFILE=ON
    # symlink beamline compact files
    printf "\nsymlink beamline to local...\n"
    rm -vf $module/ip6
    ln -svf $BEAMLINE_PATH/ip6 $module/ip6
    ;;
  NPDet)
    prefix=$module/install
    ;;
  DD4hep)
    prefix=$module/install
    genOpt DD4HEP_USE_GEANT4=ON
    genOpt DD4HEP_USE_EDM4HEP=ON
    genOpt DD4HEP_USE_HEPMC3=ON
    genOpt Boost_NO_BOOST_CMAKE=ON
    genOpt DD4HEP_USE_LCIO=ON
    genOpt BUILD_TESTING=ON
    genOpt ROOT_DIR=$ROOTSYS
    genOpt CMAKE_BUILD_TYPE=Release
    ;;
esac

########################################
# build cmake commands

# set buildsystem generation command
genOpt CMAKE_INSTALL_PREFIX=$prefix
genOpts+=$extraOpts
buildSys=$module/build
cmakeGen="cmake -S $module -B $buildSys $genOpts"

# set build command
buildOpts="-j $nproc"
# if [ $clean -eq 1 ]; then
#   buildOpts+=" --fresh" # FIXME: need cmake 3.24+; replaces `rm $buildSys` below
# fi
cmakeBuild="cmake --build $buildSys -j$nproc"

# set install command
cmakeInstall="cmake --install $buildSys"

# print
printf """
CMAKE COMMANDS:
========================================\n
[ generate buildsystem ]\n$cmakeGen\n
[ build ]\n$cmakeBuild\n
[ install ]\n$cmakeInstall\n
========================================\n
"""

########################################
# module-specific cleans
if [ $clean -eq 1 ]; then
  # remove build system
  # FIXME: delete when using `cmake --fresh`
  echo "--- CLEAN BUILDSYSTEM $buildSys"
  mkdir -p $buildSys
  rm -rv $buildSys
  case $module in
    epic)
      echo "--- CLEAN: rendered compact files from source directory..."
      rm -vf $module/epic*.xml
      echo "--- CLEAN: transient compact files from scripts/vary_params.rb..."
      rm -vf $module/epic_drich_variant*.xml
      rm -vf $module/compact/drich_variant*.xml
      rm -vf ${DETECTOR_PATH}/epic_drich_variant*.xml
      rm -vf ${DETECTOR_PATH}/compact/drich_variant*.xml
      ;;
  esac
fi

########################################
# run cmake
[ $fast -eq 0 ] && $cmakeGen
$cmakeBuild
$cmakeInstall

########################################
# post-cmake tasks
function warn_env {
  printf "\nIf this is your first time installing module '$module' to prefix\n  '$prefix'\nsource environ.sh again\n\n"
}
case $module in
  epic)
    warn_env
    ;;
  EICrecon)
    warn_env
    ;;
  athena)
    # make legacy prefix compatible with EPIC expectations
    printf "\ncompatibility updates for built targets...\n"
    rm -vf $DETECTOR_PATH/ip6
    ln -svf $BEAMLINE_PATH/ip6 $DETECTOR_PATH/ip6
    ln -svf $DETECTOR_PATH/compact/subsystem_views/drich_only.xml $DETECTOR_PATH/${DETECTOR}_drich_only.xml
    ;;
  NPDet)
    printf "\nDone. To use, run:  source scripts/this_NPDet.sh\n\n"
    printf "Troubleshooting: try instead to directly call scripts in NPDet/install/bin\n\n"
    ;;
  DD4hep)
    printf "\nDone. To use, run:  source scripts/this_DD4hep.sh\n\n"
    ;;
esac
