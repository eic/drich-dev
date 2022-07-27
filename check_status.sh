#!/bin/bash
# runs 'git status' in each repository
wd=$(pwd)
echo ""
for repo in . ip6 epic eicd irt juggler reconstruction_benchmarks; do
  cd $repo
  echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  $(basename `pwd`)"
  git status
  cd $wd
  echo ""
  echo ""
done
