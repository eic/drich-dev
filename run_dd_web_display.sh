#!/bin/bash
set -e

# default output file
geoDir=$(pwd)/geo
outputFile=$geoDir/detector_geometry.root
mkdir -p $geoDir

# set compact file, depending on (optional) argument; default is full epic
case $1 in
  d)
    compactFile="epic_drich_only.xml"
    ;;
  p)
    compactFile="epic_pfrich_only.xml"
    ;;
  c)
    if [ $# -lt 3 ]; then
      echo "ARG 'c' requires <compactFile> and <outputFile>"
      exit 2
    fi
    compactFile=$(echo $2 | sed 's;^epic/;;')
    outputFile=$(pwd)/$3
    ;;
  *)
    compactFile="epic.xml"
esac
echo "compactFile = $compactFile"
echo "outputFile  = $outputFile"

# produce geometry root file
pushd epic
dd_web_display --export -o $outputFile $compactFile
popd
echo ""
echo "produced $outputFile"
echo " -> open it with jsroot to view the geometry"
echo ""
