#!/bin/bash
# runs 'git status' in each repository
wd=$(pwd)
for repo in irt ip6 athena eicd juggler reconstruction_benchmarks; do
  cd $repo
  echo "===============>>> $repo"
  git status
  cd $wd
  echo ""
  echo ""
done
