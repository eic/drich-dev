#!/bin/bash
# list some objects on the CI runner
echo "[CI] ls -t $(pwd)"
ls -ltp
echo "[CI] PREFIX TREE"
[ -d "prefix" ] && find prefix || echo "no prefix tree"
echo "[CI] BRANCHES"
check_branches.sh
echo "[CI] UPDATE PERMISSIONS"
[ -d "bin" ] && chmod 755 bin -Rv || echo "no bin dir"
[ -d "prefix" ] && chmod 755 prefix/bin -Rv || echo "no prefix tree"
