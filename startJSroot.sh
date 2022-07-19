#!/bin/bash

# link `geo` directory, so local HTTP server can access it
ln -svf ../geo jsroot/

# start a local server
pushd jsroot
python -m http.server
popd
