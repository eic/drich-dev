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
    def __init__(self, graph, func, fit_range, extrap_range, extrap_npoints, units='', sigfigs=[5,9,5]):
        self.graph          = graph
        self.func           = func
        self.fit_range      = fit_range
        self.extrap_range   = extrap_range
        self.extrap_npoints = extrap_npoints
        self.units          = units
        self.sigfigs        = sigfigs
        self.graph_range = [
            self.graph.GetPointX(0),
            self.graph.GetPointX(self.graph.GetN()-1)
        ]

    def extrap(self):
        # perform the fit
        if self.func is not None:
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
        if self.func is not None:
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
        if self.func is not None:
            self.func.Draw('SAME')
        # print the table
        self.table = []
        for gr in [ self.graph_extrap[0], self.graph, self.graph_extrap[1] ]:
            for i in range(gr.GetN()):
                energy = 1239.841875 / gr.GetPointX(i)
                val = gr.GetPointY(i)
                line = f'      {energy:<.{self.sigfigs[0]}f}*eV  {val:>{self.sigfigs[1]}.{self.sigfigs[2]}f}'
                if(self.units!=''):
                    line += f'*{self.units}'
                self.table.append(line)
        self.table.reverse()
        print('-'*80)
        print(f'TABLE: {self.graph.GetName()}')
        print('-'*80)
        for line in self.table:
            print(line)
        print('-'*80)

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

# replacements: replace any tables from `g4dRIChOptics`
# --------------------------------------------------------------------------

### aerogel ABSLENGTH: from ________FIXME: source needed___________
aerogel_abslength_update_table = [  # [nm] [mm]
    [ 890, 58.661475  ],
    [ 880, 58.6551    ],
    [ 870, 58.64805   ],
    [ 860, 58.640225  ],
    [ 850, 58.631525  ],
    [ 840, 58.6219    ],
    [ 830, 58.611125  ],
    [ 820, 58.599175  ],
    [ 810, 58.585825  ],
    [ 800, 58.570925  ],
    [ 790, 58.554275  ],
    [ 780, 58.535575  ],
    [ 770, 58.51465   ],
    [ 760, 58.49115   ],
    [ 750, 58.464675  ],
    [ 740, 58.4349    ],
    [ 730, 58.4013    ],
    [ 720, 58.363375  ],
    [ 710, 58.320375  ],
    [ 700, 58.27175   ],
    [ 690, 58.216525  ],
    [ 680, 58.153725  ],
    [ 670, 58.0823    ],
    [ 660, 58.0008    ],
    [ 650, 57.907675  ],
    [ 640, 57.801175  ],
    [ 630, 57.67915   ],
    [ 620, 57.539075  ],
    [ 610, 57.378125  ],
    [ 600, 57.19285   ],
    [ 590, 56.979225  ],
    [ 580, 56.7327    ],
    [ 570, 56.447825  ],
    [ 560, 56.118275  ],
    [ 550, 55.7368    ],
    [ 540, 55.294975  ],
    [ 530, 54.78305   ],
    [ 520, 54.19      ],
    [ 510, 53.503475  ],
    [ 500, 52.709575  ],
    [ 490, 51.79315   ],
    [ 480, 50.73805   ],
    [ 470, 49.52745   ],
    [ 460, 48.14475   ],
    [ 450, 46.574425  ],
    [ 440, 44.8035    ],
    [ 430, 42.8232    ],
    [ 420, 40.630825  ],
    [ 410, 38.23185   ],
    [ 400, 35.641575  ],
    [ 390, 32.886125  ],
    [ 380, 30.00305   ],
    [ 370, 27.039875  ],
    [ 360, 24.05185   ],
    [ 350, 21.098575  ],
    [ 340, 18.239375  ],
    [ 330, 15.529475  ],
    [ 320, 13.0157    ],
    [ 310, 10.7339625 ],
    [ 300, 8.7075725  ],
    [ 290, 6.9466825  ],
    [ 280, 5.44934    ],
    [ 270, 4.2030425  ],
    [ 260, 3.1872325  ],
    [ 250, 2.3760525  ],
    [ 240, 1.7410525  ],
    [ 230, 1.2535325  ],
    [ 220, 0.88632675 ],
    [ 210, 0.61495675 ],
    [ 200, 0.4182277  ],
]
aerogel_abslength_update_table.reverse()
aerogel_abslength_update_graph = r.TGraphErrors()
aerogel_abslength_update_graph.SetName("graph_Aerogel_ABSLENGTH__updated")
aerogel_abslength_update_graph.SetTitle("Aerogel ABSLENGTH [mm]")
aerogel_abslength_update_graph.SetMarkerColor(r.kBlue)
aerogel_abslength_update_graph.SetMarkerStyle(r.kFullCircle)
for wl, a in aerogel_abslength_update_table:
    aerogel_abslength_update_graph.AddPoint(wl, a)

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
        [10, 10],
        '',
        [5, 7, 5]
        )
tabs['aerogel']['rindex'].set_fake_errors(3e-5)

### aerogel - ABSLENGTH (OLD, CLAS12-based)
### - linear fit to 350 nm and above only
# tabs['aerogel']['abslength_old'] = MPT(
#         root_file.Get('graph_Aerogel_ABSLENGTH'),
#         r.TF1("aerogel_abslength", "[0]+[1]*x", 350, FULL_WAVELENGTH_RANGE[-1]),
#         [350, 600],
#         FULL_WAVELENGTH_RANGE,
#         [0, 10],
#         'mm',
#         [5, 7, 3]
#         )
# tabs['aerogel']['abslength_old'].set_fake_errors(0.5)

### aerogel - ABSLENGTH (UPDATED, from above)
### - linear fit to the last few points
tabs['aerogel']['abslength'] = MPT(
        aerogel_abslength_update_graph,
        r.TF1("aerogel_abslength", "[0]+[1]*x", 870, FULL_WAVELENGTH_RANGE[-1]),
        [870, 890],
        FULL_WAVELENGTH_RANGE,
        [0, 11],
        'mm',
        [5, 7, 3]
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
        [0, 10],
        'mm',
        [5, 7, 3]
        )
tabs['aerogel']['rayleigh'].set_fake_errors(4)


# extrapolate
# --------------------------------------------------------------------------
for obj_name, obj in tabs.items():
    for tab_name, tab in obj.items():
        tab.extrap()
