#!/usr/bin/env python
# Copyright 2023, Christopher Dilks
# Subject to the terms in the LICENSE file found in the top-level directory.

# extrapolate some of the material property tables to a broader range

import ROOT as r
import sys

root_file = r.TFile.Open('out/optical_materials_drich.root', 'READ')

# Material Propery Table class, to hold graphs, fit function, etc.
# --------------------------------------------------------------------------

class MPT:
    def __init__(self, graph, func, fit_range, extrap_range):
        self.graph        = graph
        self.func         = func
        self.fit_range    = fit_range
        self.extrap_range = extrap_range

    def extrap(self):
        # perform the fit
        print(f'Fit graph "{self.graph.GetName()}" to function:')
        self.func.Print()
        self.graph.Fit(self.func, '', '', self.fit_range[0], self.fit_range[1])
        # extrapolate
        self.graph_extrap = r.TGraph()
        self.graph_extrap.SetName(self.graph.GetName())
        self.graph_extrap.SetTitle(self.graph.GetTitle())
        #
        # TODO
        #
        # draw the results
        self.graph.Draw('APE')
        self.func.Draw('SAME')

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

# aerogel - RINDEX
# - fit to 2nd order Sellmeier function
tabs['aerogel'] = {}
sellmeier = r.TF1("aerogel_rindex", make_sellmeier(2), 0, 1000)
sellmeier.SetParLimits(1,0.01,400)
tabs['aerogel']['rindex'] = MPT(
        root_file.Get('graph_Aerogel_RINDEX'),
        sellmeier,
        [200, 650],
        [100, 1000]
        )
tabs['aerogel']['rindex'].set_fake_errors(0.0001)


# extrapolate
# --------------------------------------------------------------------------
for obj_name, obj in tabs.items():
    for tab_name, tab in obj.items():
        tab.extrap()
