#!/bin/bash
#
# test run of the full simulation and reconstruction pipeline;
# this is similar to the CI workflow in EICrecon
#

config=arches
num_events=20
sim_file=out/sim_full_${config}.edm4hep.root
rec_file=out/rec_full_${config}

### simulation
npsim --compactFile ${DETECTOR_PATH}/${DETECTOR}_${config}.xml \
  -G --gun.particle "pi-" \
  --gun.momentumMin "1*GeV" \
  --gun.momentumMax "20*GeV" \
  --gun.distribution "uniform" \
  -N 20 \
  --outputFile $sim_file

### reconstruction (EICrecon)
export DETECTOR_CONFIG=${DETECTOR}_${config}
run_eicrecon_reco_flags.py $sim_file $rec_file.eicrecon \
  -Peicrecon:LogLevel=trace

### reconstruction (Juggler)
export JUGGLER_SIM_FILE=$sim_file
export JUGGLER_REC_FILE=$rec_file.juggler.tree.edm4eic.root
export JUGGLER_N_EVENTS=$num_events
options_file=/opt/benchmarks/physics_benchmarks/options/reconstruction.py
gaudirun.py $options_file

### info
echo """
OUTPUTS:
       simulation file: $sim_file
  reconstruction files: $rec_file.eicrecon.tree.edm4eic.root
                        $rec_file.juggler.tree.edm4eic.root
"""
