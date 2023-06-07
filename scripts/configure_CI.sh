#!/bin/bash
# configuration for the CI runner
echo "[CI] ls -t $(pwd)"
ls -ltp
echo "[CI] PREFIX TREE"
[ -d "prefix" ] && find prefix || echo "no prefix tree"
echo "[CI] BRANCHES"
check_branches.sh
