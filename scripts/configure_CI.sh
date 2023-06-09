#!/bin/bash
# configuration for the CI runner

# clone repositories
repo_list="epic EDM4eic irt EICrecon reconstruction_benchmarks"
if [ $# -gt 0 ]; then repo_list=$*; fi
echo "[CI] CLONING REPOSITORIES: $repo_list"
for repo in $repo_list; do
  case $repo in
    epic)
      echo git clone https://github.com/eic/epic.git --branch main
      ;;
    EDM4eic)
      echo git clone https://github.com/eic/EDM4eic.git --branch main
      ;;
    irt)
      echo git clone https://github.com/eic/irt.git --branch main
      ;;
    EICrecon)
      echo git clone https://github.com/eic/EICrecon.git --branch irt-algo
      ;;
    reconstruction_benchmarks)
      echo git clone https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks.git --branch irt-algo
      ;;
    none)
      echo "Not cloning any repositories"
      ;;
    *)
      echo "ERROR: unknown repository '$repo'"
      exit 1
      ;;
  esac
done
exit

# list files in the current directory
echo "[CI] ls -t $(pwd)"
ls -ltp

# list files in the installation tree
echo "[CI] PREFIX TREE"
[ -d "prefix" ] && find prefix || echo "no prefix tree"

# check git branches
echo "[CI] BRANCHES"
check_branches.sh
