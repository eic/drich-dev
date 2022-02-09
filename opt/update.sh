#!/bin/bash
# install or update the EIC software container; basically
# a wrapper for `eic_container/install.sh`

set -e

# cd to opt directory
if [ -z "${BASH_SOURCE[0]}" ]; then
  opt=$(dirname $(realpath $0))
else
  opt=$(dirname $(realpath ${BASH_SOURCE[0]}))
fi
pushd $opt

# update `eic_container` repo
if [ -d "eic_container" ]; then
  echo "[+] update eic_container repo..."
  pushd eic_container
  git pull
  popd
else
  echo "[+] clone eic_container repo..."
  git clone https://eicweb.phy.anl.gov/containers/eic_container.git
fi

# pull the container
echo "[+] execute install.sh..."
eic_container/install.sh $*
popd
