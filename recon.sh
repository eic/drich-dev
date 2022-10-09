#!/bin/bash

### FIXME: implement as an option
eicrecon \
  -Pplugins=irtgeo,irt,RICH \
  -Ppodio:output_include_collections=IRT \
  out/sim.root
exit $?
#################################

# default settings
which_rich=""
method=""
sim_file="out/sim.root"
rec_file="out/rec.root"
aux_file=""
compact_file_base="${DETECTOR_PATH}/${DETECTOR}"
compact_file=""
use_full=0
dry_run=0

# create file names, given detector
function compact_only_name { echo "${compact_file_base}_${1}_only.xml"; }
function compact_full_name { echo "${compact_file_base}.xml"; }
function aux_name { echo "geo/irt-${1}.root"; }

# usage
function usage {
  echo """
USAGE:
  $0 [DETECTOR] [RECONSTRUCTION METHOD] [OPTION]...

  DETECTOR (required)
    -d  use dRICH data for reconstruction
    -p  use pfRICH data for reconstruction

  RECONSTRUCTION METHODS: (one required)
    -j  run reconstruction through juggler
    -r  run reconstruction with stand-alone IRT reader.C macro

  OPTIONS
    -i <simulation_output_file>
        Input to the reconstruction: ROOT file from DD4hep + Geant4
        [ default = $sim_file ]
    -o <reconstruction_output_file>
        Output ROOT file
        [ default = $rec_file ]
    -x <irt_auxfile>
        IRT geometry auxiliary config file
        [ default (depends on which RICH):
            $(aux_name drich)
            $(aux_name pfrich)
        ]
    -c <compact_file>
        Custom top-level compact file for DD4hep geometry
        [ default (depends on which RICH):
            $(compact_only_name drich)
            $(compact_only_name pfrich)
        ]
    -f  Use the full detector's compact file (likely with B-field):
            $(compact_full_name)
        [ default: use the standalone RICH, and no B-field ]
    -t  Test using a dry-run; just prints settings
  """
  exit 2
}
if [ $# -lt 2 ]; then usage; fi

# parse options
while getopts "hdpjri:o:x:c:ft" opt; do
  case $opt in
    h|\?) usage             ;;
    d) which_rich="drich"   ;;
    p) which_rich="pfrich"  ;;
    j) method="juggler"     ;;
    r) method="reader"      ;;
    i) sim_file=$OPTARG     ;;
    o) rec_file=$OPTARG     ;;
    x) aux_file=$OPTARG     ;;
    c) compact_file=$OPTARG ;;
    f) use_full=1           ;;
    t) dry_run=1            ;;
  esac
done

# set default rich-dependent settings, if unspecified
if [ -z "$which_rich" ]; then
  echo "ERROR: specify [DETECTOR]"
  usage
  exit 2
fi
[ -z "$aux_file" ]     && aux_file=$(aux_name $which_rich)
[ -z "$compact_file" ] && compact_file=$(compact_only_name $which_rich)
[ $use_full -eq 1 ]    && compact_file=$(compact_full_name)

# print settings
echo """
method       = $method
sim_file     = $sim_file
rec_file     = $rec_file
aux_file     = $aux_file
compact_file = $compact_file
use_full     = $use_full
"""

# run reconstruction
case $method in
  juggler)
    export JUGGLER_SIM_FILE=$sim_file
    export JUGGLER_REC_FILE=$rec_file
    export JUGGLER_IRT_AUXFILE=$aux_file
    export JUGGLER_COMPACT_FILE=$compact_file
    options_file=juggler/JugPID/tests/options/${which_rich}.py
    echo "RUN JUGGLER with options file $options_file"
    if [ $dry_run -eq 0 ]; then
      gaudirun.py $options_file
      printf "\nJuggler IRTAlgorithm finished\n -> produced $rec_file\n"
    fi
    ;;
  reader)
    echo "RUN STANDALONE reader macro"
    if [ $dry_run -eq 0 ]; then
      root -b -q irt/scripts/reader.C'("'$sim_file'","'$aux_file'")'
    fi
    ;;
  *)
    echo "ERROR: unspecified reconstruction method" >&2
    usage
    ;;
esac

[ $dry_run -eq 1 ] && echo "DRY RUN complete, remove '-t' to run for real"
