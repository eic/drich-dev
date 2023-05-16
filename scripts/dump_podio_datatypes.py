#!/usr/bin/env python
# dump simulation output TBranch names, together with their PODIO data type

import sys, ROOT
infile_name = 'out/sim.edm4hep.root' if len(sys.argv)<=1 else sys.argv[1]
infile = ROOT.TFile(infile_name)
tr = infile.Get("metadata")
tr.SetScanField(0)
tr.Scan("*","","colsize=30")
