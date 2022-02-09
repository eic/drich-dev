#!/bin/bash
# return parameter value(s) from compact files, given search term

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [search term (case sensitive)]"
  exit 1
fi

source environ.sh

pushd $DRICH_DD4_ATHENA
npdet_info search $1 --value athena.xml
popd
