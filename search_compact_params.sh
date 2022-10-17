#!/bin/bash
# return parameter value(s) from compact files, given search term
set -e

# check environment
if [ -z "$DETECTOR_PATH" ]; then
  echo "ERROR: source environ.sh first"
  exit 1
fi

# usage
function usage {
  echo """
USAGE:
  $0 [CONFIGURATION]

  CONFIGURATIONS: (one required)
    -e  default EPIC detector
    -a  arches (mRICH)
    -b  brycecanyon (pfRICH)
    -c <compact_file>
        custom compact file
     
    For -e,-a,-b, compact files are assumed to be at
     \$DETECTOR_PATH = $DETECTOR_PATH
     (rendered by build.sh epic)

  Use grep to search the output

  """
  exit 2
}
if [ $# -eq 0 ]; then usage; fi

# parse options
while getopts "heabc:" opt; do
  case $opt in
    h|\?) usage ;;
    e) compactFile="${DETECTOR_PATH}/${DETECTOR}.xml" ;;
    a) compactFile="${DETECTOR_PATH}/${DETECTOR}_arches.xml" ;;
    b) compactFile="${DETECTOR_PATH}/${DETECTOR}_brycecanyon.xml" ;;
    c) compactFile=$OPTARG ;;
  esac
done
echo """
compactFile = $compactFile
"""
if [ -z "$compactFile" ]; then
  echo "ERROR: specify CONFIGURATION"
  usage
  exit 1
fi

# dump table
npdet_info dump $compactFile
