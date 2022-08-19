#!/bin/bash

# default settings
method=""
sim_file="out/sim.root"
rec_file="out/rec.root"
aux_file="geo/irt-drich.root"

# usage
function usage {
  echo """
USAGE:
  $0 [RECONSTRUCTION METHOD] [OPTION]...

  RECONSTRUCTION METHODS: (one required)
    -j  run reconstruction through juggler
    -r  run reconstruction with stand-alone reader macro

  OPTIONS
    -i <simulation_output_file>
        Input to the reconstruction: ROOT file from DD4hep + Geant4
        [ default = $sim_file ]
    -o <reconstruction_output_file>
        Output ROOT file
        [ default = $rec_file ]
    -x <irt_auxfile>
        IRT geometry auxiliary config file
        [ default = $aux_file ]
  """
  exit 2
}
if [ $# -eq 0 ]; then usage; fi

# parse options
while getopts "hjri:o:x:" opt; do
  case $opt in
    h|\?) usage ;;
    j) method="juggler" ;;
    r) method="reader"  ;;
    i) sim_file=$OPTARG ;;
    o) rec_file=$OPTARG ;;
    x) aux_file=$OPTARG ;;
  esac
done
echo """
method   = $method
sim_file = $sim_file
rec_file = $rec_file
aux_file = $aux_file
"""

# run reconstruction
case $method in
  juggler)
    export JUGGLER_SIM_FILE=$sim_file
    export JUGGLER_REC_FILE=$rec_file
    export JUGGLER_IRT_AUXFILE=$aux_file
    gaudirun.py juggler/JugPID/tests/options/irt.py
    printf "\nJuggler IRTAlgorithm finished\n -> produced $rec_file\n"
    ;;
  reader)
    root -b -q irt/scripts/reader_2.C'("'$sim_file'","'$aux_file'")'
    ;;
  *)
    echo "ERROR: unspecified reconstruction method" >&2
    usage
    ;;
esac
