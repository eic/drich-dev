#!/bin/bash
# cmake wrapper to build juggler
# - assumes you've symlinked or cloned juggler to `./juggler`
# - build dir will be juggler/build
# - don't run too many parallel jobs
#
# - nasty kluge warning: the `sed -i ... $klugedFile` commands work around
#   `/usr/local/listcomponent`'s stubborn preference of "installed" libraries
#   (e.g., in `$LD_LIBRARY_PATH`) over those we have just built; the regexp
#   forces `listcomponents` to use the newly built library in the `pwd`; since
#   this will write unecessary `./` characters to the output `*.components` file,
#   a second regexp is used to remove them
klugedFile=build/JugPID/CMakeFiles/JugPIDPlugins.dir/build.make

pushd juggler
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=$JUGGLER_INSTALL_PREFIX && \
sed -i '/listcomponents/{s/ libJug/ .\/libJug/;}' $klugedFile && \
sed -i 's/listcomponents.*$/& \&\& sed -i "s\/\\\.\\\/\/\/g" JugPIDPlugins.components/g' $klugedFile && \
cmake --build build -j2 -- install && \
popd && exit 0
popd
exit 1
