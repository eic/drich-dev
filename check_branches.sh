#!/bin/bash
# check which branch you are on for each repository
wd=$(pwd)
echo ""
for repo in . epic EDM4eic irt juggler reconstruction_benchmarks; do
  cd $repo
  printf "%30s: %s  (%s)\n" $(basename `pwd`) $(git branch | awk '/^\*/{print $2}') $(git rev-parse --short HEAD)
  cd $wd
done
echo ""
