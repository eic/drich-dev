#!/bin/bash
set -e

# check environment
if [ -z "$DETECTOR_PATH" ]; then
  echo "ERROR: source environ.sh first"
  exit 1
fi

# default output file
geoDir=geo
outputFile=$geoDir/detector_geometry.root
mkdir -p $geoDir

# usage
function usage {
  echo """
USAGE:
  $0 [MODE] [OPTIONS]...

  MODES: (one required)
    -d  dRICH only
    -p  pfRICH only
    -m  mRICH only
    -e  full EPIC detector
    -a  full EPIC detector, arches configuration
    -b  full EPIC detector, brycecanyon configuration
    -c <compact_file>
        custom compact file
     
    For -e,-d,-p, compact files are assumed to be at
     \$DETECTOR_PATH = $DETECTOR_PATH
     (rendered by build.sh epic)

  OPTIONS
    -o <output_root_file>
        Output ROOT file with TGeo geometry (view with jsroot)
        [ default = $outputFile ]

  """
  exit 2
}
if [ $# -eq 0 ]; then usage; fi

# parse options
while getopts "hdpmeabc:o:" opt; do
  case $opt in
    h|\?) usage ;;
    d) compactFile="${DETECTOR_PATH}/${DETECTOR}_drich_only.xml" ;;
    p) compactFile="${DETECTOR_PATH}/${DETECTOR}_pfrich_only.xml" ;;
    m) compactFile="${DETECTOR_PATH}/${DETECTOR}_mrich_only.xml" ;;
    e) compactFile="${DETECTOR_PATH}/${DETECTOR}.xml" ;;
    a) compactFile="${DETECTOR_PATH}/${DETECTOR}_arches.xml" ;;
    b) compactFile="${DETECTOR_PATH}/${DETECTOR}_brycecanyon.xml" ;;
    c) compactFile=$OPTARG ;;
    o) outputFile=$OPTARG ;;
  esac
done
echo """
compactFile = $compactFile
outputFile  = $outputFile
"""
if [ -z "$compactFile" ]; then
  echo "ERROR: specify MODE"
  usage
  exit 1
fi

# produce geometry root file
geoConverter -compact2tgeo -input $compactFile -output $outputFile
echo ""
echo "produced $outputFile"
echo " -> open it with jsroot to view the geometry"
echo ""
