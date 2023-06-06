#!/bin/bash
# change the optics debugging modes on-the-fly

if [ $# -ne 2 ]; then
  echo "USAGE: $0 [constant_name] [value]"
  exit 2
fi
compact='epic/compact/pid/drich.xml'
pattern='constant.*'$1'.*value="'

echo "before:"
grep -E $pattern $compact

sed -i 's;\('$pattern'\)\(.*"\);\1'$2'";' $compact

echo "after:"
grep -E $pattern $compact

echo "done; now re-build epic"
