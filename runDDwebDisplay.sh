#!/bin/bash
set -e

# set compact file, depending on (optional) argument; default is full ecce
compactFile=$(
  case "$1" in
    ("d") echo "compact/subsystem_views/drich_only.xml"  ;;
    ("p") echo "compact/subsystem_views/pfrich_only.xml" ;;
    (*)   echo "ecce.xml"                                ;;
  esac)
echo "compactFile = $compactFile"

# produce geometry root file
wdir=$(pwd)
pushd ecce
dd_web_display --export -o $wdir/detector_geometry.root $compactFile
popd
echo "produced $(ls -t *.root|head -n1), view with jsroot"
