#!/bin/bash
# prioritize local DD4hep installation

# sourcing DD4hep/install/bin/thisdd4hep.sh re-orders items in these PATH variables;
# here we temporarily store them so we can restore them afterward
LD_LIBRARY_PATH_TMP=$LD_LIBRARY_PATH
PYTHONPATH_TMP=$PYTHONPATH

# source installed thisdd4hep.sh
source DD4hep/install/bin/thisdd4hep.sh

# restore PATH variables, and prepend the DD4hep installation paths
pyDir=$(find $DD4hep_ROOT/lib -type d -name "python*" -print | head -n1)
export PYTHONPATH=$pyDir/site-packages:$PYTHONPATH_TMP
export LD_LIBRARY_PATH=$DD4hep_ROOT/lib:$LD_LIBRARY_PATH_TMP

# thisdd4hep.sh also doens't select the newly installed examples
export DD4HEP=$DD4hep_ROOT/examples

# add /usr/local/bin back to $PATH (workaround..)
export PATH="/usr/local/bin:$PATH"


# old test version (not using thisdd4hep.sh)
# export DD4hep_ROOT=$DRICH_DEV/DD4hep/install
# export DD4hepINSTALL=$DD4hep_ROOT
# export DD4hep_DIR=$DD4hep_ROOT
# export DD4HEP=$DD4hep_ROOT/examples
# pyDir=$(find $DD4hep_ROOT/lib -type d -name "python*" -print | head -n1)
# export PYTHONPATH=$pyDir/site-packages:$PYTHONPATH
# # export LD_LIBRARY_PATH=$DD4hep_ROOT/lib:$LD_LIBRARY_PATH
