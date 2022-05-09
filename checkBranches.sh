#!/bin/bash
# check which branch you are on for each repository
wd=$(pwd)
for repo in irt ip6 ecce eicd juggler reconstruction_benchmarks; do
  cd $repo
  printf "%30s: %s\n" $repo $(git branch | awk '/^\*/{print $2}')
  cd $wd
done
