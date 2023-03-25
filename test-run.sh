#!/bin/bash

# source prefix2/install/bin/eicrecon-this.sh
# echo $ROOT_INCLUDE_PATH
# echo "EXIT PREMATURELY"; exit

export JANA_PLUGIN_PATH=prefix2/install/lib/EICrecon/plugins${JANA_PLUGIN_PATH:+:${JANA_PLUGIN_PATH}}
echo "JANA_PLUGIN_PATH = $JANA_PLUGIN_PATH"

# export ROOT_INCLUDE_PATH=prefix2/install/include/IRT${ROOT_INCLUDE_PATH:+:${ROOT_INCLUDE_PATH}}
# export ROOT_INCLUDE_PATH=/usr/local/include/IRT${ROOT_INCLUDE_PATH:+:${ROOT_INCLUDE_PATH}}
# echo "ROOT_INCLUDE_PATH = $ROOT_INCLUDE_PATH"

# export LD_LIBRARY_PATH=$(pwd)/prefix2/install/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
# echo $LD_LIBRARY_PATH

prefix2/install/bin/eicrecon \
  -Ppodio:output_include_collections=DRICHHits \
  -Ppodio:output_file=out/rec.root \
  -Pjana:nevents=0 \
  -Pjana:debug_plugin_loading=1 \
  -Pacts:MaterialMap=calibrations/materials-map.cbor \
  out/sim.root
