#!/bin/bash
# check which branch you are on for each repository
wd=$(pwd)
for repo in irt ip6 athena eicd juggler reconstruction_benchmarks; do
  cd $repo
  echo $repo: $(git branch | awk '/^\*/{print $2}')
  cd $wd
done | column -t
  
