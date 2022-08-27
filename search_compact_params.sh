#!/bin/bash
# return parameter value(s) from compact files, given search term

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [search term (case sensitive)]"
  exit 2
fi

if [ -z "$DETECTOR_PATH" ]; then
  echo "ERROR: source environ.sh first"
  exit 1
fi

compact_file=$DETECTOR_PATH/$DETECTOR.xml
echo "Searching compact file $compact_file"
NPDet/install/bin/npdet_info search $1 --value $compact_file
