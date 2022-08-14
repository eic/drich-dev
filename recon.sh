#!/bin/bash

# default settings
method=""
sim_file="out/irt.root"
aux_file="geo/irt-drich.root"

# usage
function usage {
  echo """
USAGE:
  $0 [OPTION]...

  RECONSTRUCTION METHODS: (one required)
    -j  run reconstruction through juggler
    -r  run reconstruction with stand-alone reader macro

  OPTIONS (for method  -r  only)
    -s <simulation_output>
        simulation output file
        default = $sim_file
    -x <irt_auxfile>
        IRT geometry auxiliary config file
        default = $aux_file
  """
  exit 2
}
if [ $# -eq 0 ]; then usage; fi

# parse options
while getopts "hjrs:x:" opt; do
  case $opt in
    h|\?) usage ;;
    j) method="juggler" ;;
    r) method="reader"  ;;
    s) sim_file=$OPTARG ;;
    x) aux_file=$OPTARG ;;
  esac
done
echo """
method   = $method
sim_file = $sim_file
aux_file = $aux_file
"""

# run reconstruction
case $method in
  juggler)
    gaudirun.py scripts/src/juggler-options.py
    ;;
  reader)
    root -b -q scripts/irt_reader.C'("'$sim_file'","'$aux_file'")'
    ;;
  *)
    echo "ERROR: unspecified reconstruction method" >&2
    usage
    ;;
esac
