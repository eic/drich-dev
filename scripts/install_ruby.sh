#!/bin/bash
# use rbenv and ruby-install to install a local copy of ruby

set -e

VERSION=3.1.2

function sep() {
  echo """
$*
++++++++++++++++++++++++++++++++++++++++"""
}

sep "install rbenv"
if [ "$RBENV_ROOT" = "" ]; then echo "ERROR: source environ.sh first"; exit 1; fi
git clone https://github.com/rbenv/rbenv.git .rbenv
# pushd .rbenv
# src/configure
# make -C src
# popd

sep "source environ"
source environ.sh

sep "install ruby-build"
mkdir -p "$(rbenv root)"/plugins
git clone https://github.com/rbenv/ruby-build.git $(rbenv root)/plugins/ruby-build

sep "install ruby version $VERSION"
export RUBY_BUILD_BUILD_PATH=.ruby
mkdir -p RUBY_BUILD_BUILD_PATH
rbenv install --verbose $VERSION
rbenv local $VERSION

sep "done; now re-run source environ.sh to enable the new ruby installation"
