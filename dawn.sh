#!/bin/bash
set -e
pushd epic
bin/generate_prim_file -o prim -D -t detector_view
mkdir -p images
bin/make_dawn_views -i prim/detector_view.prim -t view1 -d scripts/view1 -D
popd
echo "see  epic/images/view1_top.png"
