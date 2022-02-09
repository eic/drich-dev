#!/bin/bash
# run momentum scan for 4 different particles

for particle in 'e-' 'pi+' 'kaon+' 'proton'; do
  outname="out/count.old.$(echo $particle | sed 's/-//;s/+//').root"
  echo "RUN $particle output $outname"
  ./simulate.py -t7 -n100 -p "$particle" -o $outname
  ./drawHits.exe $outname
done
