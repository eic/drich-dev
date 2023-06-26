#!/bin/bash
# generate table of sensor positions and orientation vectors

table_file=geo/sensor_table.txt
root_file=geo/sensor_table.root
bin/create_irt_auxfile d | grep sensor_de | sed 's;^.*sensor_de;sensor_de;g' > $table_file
python -c """
import ROOT as r
root_file = r.TFile(\"$root_file\", 'RECREATE')
tr = r.TTree('table','table')
tr.ReadFile(\"$table_file\", \"name/C:ID/C:sector/I:pos_x/D:pos_y/D:pos_z/D:normX_x/D:normX_y/D:normX_z/D:normY_x/D:normY_y/D:normY_z/D\")
tr.Write()
root_file.Close()
"""
echo """
CREATED:
  $table_file
  $root_file
"""
