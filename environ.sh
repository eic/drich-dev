# paths
# note: juggler and athena share the same install prefix ...bad idea? We shall see...
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ATHENA_PREFIX/lib
export JUGGLER_DETECTOR=athena
export DETECTOR_PATH=$(pwd)/athena
export DETECTOR_VERSION=master
export JUGGLER_INSTALL_PREFIX=$ATHENA_PREFIX

# default settings 
export JUGGLER_SIM_FILE=$(pwd)/sim/sim_run.root
export JUGGLER_REC_FILE=test.root
export JUGGLER_N_EVENTS=100
