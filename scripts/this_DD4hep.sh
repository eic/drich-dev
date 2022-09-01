#!/bin/bash
# prioritize local DD4hep installation

# source DD4hep/install/bin/thisdd4hep.sh
# export DD4HEP=$DD4hep_ROOT/examples

export DD4hep_ROOT=$DRICH_DEV/DD4hep/install
export DD4hepINSTALL=$DD4hep_ROOT
export DD4hep_DIR=$DD4hep_ROOT
export DD4HEP=$DD4hep_ROOT/examples
pyDir=$(find $DD4hep_ROOT/lib -type d -name "python*" -print | head -n1)
export PYTHONPATH=$pyDir/site-packages:$PYTHONPATH
export LD_LIBRARY_PATH=$DD4hep_ROOT/lib:$LD_LIBRARY_PATH
