#!/bin/bash

# default settings
method=""

# usage
function usage {
  echo """
USAGE:
  $0 [OPTION]...

    -j
        run reconstruction through juggler
    -r
        run reconstruction with stand-alone reader macro
  """
  exit 2
}

# parse options
while getopts "hjr" opt; do
  case $opt in
    h|\?)
      usage
      ;;
    j)
      method="juggler"
      ;;
    r)
      method="reader"
      ;;
  esac
done
echo """
reconstruction method: $method
"""

# run reconstruction
case $method in
  juggler)
    gaudirun.py scripts/src/juggler-options.py
    ;;
  reader)
    echo "reader"
    ;;
  *)
    echo "ERROR: unspecified reconstruction method" >&2
    usage
    ;;
esac
