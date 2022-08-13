#!/bin/bash
# runs 'git status' in each repository
wd=$(pwd)
echo ""
for repo in . epic eicd irt reconstruction_benchmarks; do
  cd $repo
  echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  $(basename `pwd`)"
  git status
  cd $wd
  echo ""
  echo ""
done
