#!/bin/bash
# evaluation of reconstruction performance

# default settings
rec_file="out/rec.root"
out_file="out/eval.root"

# usage
function usage {
  echo """
USAGE:
  $0 [OPTION]...

  OPTIONS
    -i <reconstruction_output_file>
        ROOT file produced by reconstruction
        [ default = $rec_file ]
    -o <output_file>
        Evaluation output ROOT file
        [ default = $out_file ]
  """
  exit 2
}

# parse options
while getopts "hi:o:" opt; do
  case $opt in
    h|\?) usage ;;
    i) rec_file=$OPTARG ;;
    o) out_file=$OPTARG ;;
  esac
done
echo """
rec_file = $rec_file
out_file = $out_file
"""

# call IRT evaluation script
root -b -q irt/scripts/evaluation.C'("'$rec_file'","'$out_file'")'
