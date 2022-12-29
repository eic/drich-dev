#!/bin/bash
# runs 'git status' in each repository
wd=$(pwd)
echo ""
for repo in . epic EDM4eic irt EICrecon juggler reconstruction_benchmarks; do
  if [ -d "$repo" ]; then
    cd $repo
    echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  $(basename `pwd`)"
    git status
    cd $wd
    echo ""
    echo ""
  fi
done
