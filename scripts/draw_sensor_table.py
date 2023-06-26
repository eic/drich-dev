#!/usr/bin/env python
# draw sensor positions and orientations from table produced by `make_sensor_table.sh`

import sys
import ROOT as r

root_file_name = sys.argv[1] if len(sys.argv)>1 else 'geo/sensor_table.root'
print(f'Opening file {root_file_name}...')
root_file = r.TFile(root_file_name, 'READ')
tr = root_file.Get('table')

cut = 'sector==0'

def branch3D(name):
    return f'{name}_y:{name}_x:{name}_z'
def cross(a,b):
    cx = f'{a}_y*{b}_z-{a}_z*{b}_y'
    cy = f'{a}_z*{b}_x-{a}_x*{b}_z'
    cz = f'{a}_x*{b}_y-{a}_y*{b}_x'
    return f'{cy}:{cx}:{cz}'

canv = r.TCanvas()
canv.Divide(2,2)
canv.cd(1)
tr.Draw(branch3D('pos'), cut)
canv.cd(2)
tr.Draw(branch3D('normX'), cut)
canv.cd(3)
tr.Draw(branch3D('normY'), cut)
canv.cd(4)
tr.Draw(cross('normX','normY'), cut) # normZ = normX x normY

root_file.Close()

