#!/usr/bin/env python
# Copyright 2023, Christopher Dilks
# Subject to the terms in the LICENSE file found in the top-level directory.

# extrapolate some of the material property tables to a broader range

import ROOT as r
import sys
from numpy import linspace

##### SETTINGS #########################
FULL_WAVELENGTH_RANGE = [200, 1000]
########################################

root_file = r.TFile.Open('out/optical_materials_drich.root', 'READ')

# Material Propery Table class, to hold graphs, fit function, etc.
# --------------------------------------------------------------------------

class MPT:
    def __init__(self, graph, func, fit_range, extrap_range, extrap_npoints):
        self.graph          = graph
        self.func           = func
        self.fit_range      = fit_range
        self.extrap_range   = extrap_range
        self.extrap_npoints = extrap_npoints
        self.graph_range = [
            self.graph.GetPointX(0),
            self.graph.GetPointX(self.graph.GetN()-1)
        ]

    def extrap(self):
        # perform the fit
        print(f'Fit graph "{self.graph.GetName()}" to function:')
        self.func.Print()
        self.graph.Fit(self.func, '', '', self.fit_range[0], self.fit_range[1])
        # extrapolate
        self.multi_gr = r.TMultiGraph()
        self.multi_gr.SetName(self.graph.GetName()+"_multi_gr")
        self.multi_gr.SetTitle(self.graph.GetTitle())
        self.multi_gr.Add(self.graph)
        self.graph_extrap = [ r.TGraphErrors(), r.TGraphErrors() ]
        self.graph_extrap[0].SetName(self.graph.GetName()+"_extrap_low")
        self.graph_extrap[1].SetName(self.graph.GetName()+"_extrap_high")
        self.graph_extrap[0].SetMarkerColor(r.kRed)
        self.graph_extrap[1].SetMarkerColor(r.kGreen+1)
        for i in range(2):
            self.graph_extrap[i].SetTitle(self.graph.GetTitle())
            self.graph_extrap[i].SetMarkerStyle(r.kStar)
            if( ( i==0 and self.extrap_range[i]<self.graph_range[i] ) or ( i==1 and self.extrap_range[i]>self.graph_range[i] )):
                self.multi_gr.Add(self.graph_extrap[i])
                extrap_points = list(linspace(self.extrap_range[i], self.graph_range[i], self.extrap_npoints[i]+1))
                del extrap_points[-1]
                if( i==1 ):
                    extrap_points.reverse()
                for x in extrap_points:
                    self.graph_extrap[i].AddPoint(x, self.func.Eval(x))
        # draw the results
        canv_name = f'{self.graph.GetName()}_canv'
        self.canv = r.TCanvas(canv_name, canv_name, 1000, 800)
        self.canv.SetGrid(1,1)
        self.multi_gr.Draw('APE')
        self.func.Draw('SAME')
        # print the table
        self.table = []
        for gr in [ self.graph_extrap[0], self.graph, self.graph_extrap[1] ]:
            for i in range(gr.GetN()):
                energy = 1239.841875 / gr.GetPointX(i)
                val = gr.GetPointY(i)
                self.table.append(f'  {energy:.5f}*eV   {val:.5f}')
        self.table.reverse()
        print(f'TABLE: {self.graph.GetName()}')
        for line in self.table:
            print(line)

    def set_fake_errors(self, err):
        for i in range(self.graph.GetN()):
            self.graph.SetPointError(i, 0.0, err)

tabs = {}

# fit function formula factories
# --------------------------------------------------------------------------

def make_chebyshev(order):
    chebyshev = {
            0: "1",
            1: "x",
            2: "2*x^2-1",
            3: "4*x^3-3*x",
            4: "8*x^4-8*x^2+1",
            5: "16*x^5-20*x^3+5*x",
            6: "32*x^6-48*x^4+18*x^2-1",
            7: "64*x^7-112*x^5+56*x^3-7*x",
            }
    formula = []
    for i in range(order+1):
        formula.append(f'[{i}]*({chebyshev[i]})')
    return '+'.join(formula)

def make_sellmeier(order):
    formula = ["1"]
    p = 0
    for i in range(order):
        formula.append(f'[{p}]*x^2/(x^2-[{p+1}]^2)')
        p += 2
    return f'sqrt({"+".join(formula)})'

# fits
# --------------------------------------------------------------------------

### aerogel - RINDEX
### - fit to 2nd order Sellmeier function
tabs['aerogel'] = {}
aerogel_rindex_fn = r.TF1("aerogel_rindex", make_sellmeier(2), *FULL_WAVELENGTH_RANGE)
aerogel_rindex_fn.SetParLimits(1,0.01,400)
tabs['aerogel']['rindex'] = MPT(
        root_file.Get('graph_Aerogel_RINDEX'),
        aerogel_rindex_fn,
        [200, 650],
        FULL_WAVELENGTH_RANGE,
        [10, 10]
        )
tabs['aerogel']['rindex'].set_fake_errors(3e-5)

### aerogel - ABSLENGTH
### - linear fit to 350 nm and above only
tabs['aerogel']['abslength'] = MPT(
        root_file.Get('graph_Aerogel_ABSLENGTH'),
        r.TF1("aerogel_abslength", "[0]+[1]*x", 350, FULL_WAVELENGTH_RANGE[-1]),
        [350, 600],
        FULL_WAVELENGTH_RANGE,
        [0, 10]
        )
tabs['aerogel']['abslength'].set_fake_errors(0.5)

### aerogel - RAYLEIGH
### - fit to lambda^4 dependence
aerogel_rayleigh_fn = r.TF1("aerogel_rayleigh", "[0]+[1]*x^4", *FULL_WAVELENGTH_RANGE)
tabs['aerogel']['rayleigh'] = MPT(
        root_file.Get('graph_Aerogel_RAYLEIGH'),
        aerogel_rayleigh_fn,
        [350, 600],
        FULL_WAVELENGTH_RANGE,
        [0, 10]
        )
tabs['aerogel']['rayleigh'].set_fake_errors(4)





# extrapolate
# --------------------------------------------------------------------------
for obj_name, obj in tabs.items():
    for tab_name, tab in obj.items():
        tab.extrap()
