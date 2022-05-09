#!/bin/bash
# return parameter value(s) from compact files, given search term

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [search term (case sensitive)]"
  exit 2
fi

npdet_info search $1 --value ecce/ecce.xml
