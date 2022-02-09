#!/bin/bash
# run update.sh with extra args for dilks's computer arcturus;
# these args are passed directly to eic_container/install.sh

set -e

if [ -z "${BASH_SOURCE[0]}" ]; then
  opt=$(dirname $(realpath $0))
else
  opt=$(dirname $(realpath ${BASH_SOURCE[0]}))
fi

pushd $opt
update.sh -t /home/dilks/tmp -n
popd
