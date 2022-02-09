#!/usr/bin/env python

# scan through detector parameter space, and run npsim

import numpy as np 
import math as m
import sys, os, json, subprocess, shlex, re

# get original compact file
athenaDir = os.environ['DRICH_DD4_ATHENA']
compactInName = 'compact/drich_orig.xml'
compactInFile = open(athenaDir+'/'+compactInName,'r')

# lists
def sep(): print('-'*40)
def dictPrint(d): print(json.dumps(d,indent=4))
it = {}
i=0
it['tuneX1'] = list(np.linspace( 50, 80, 4 ))
it['tuneX2'] = list(np.linspace( 50, 80, 4 ))
it['tuneZ1'] = list(np.linspace( -100, -40, 4 ))
it['tuneZ2'] = list(np.linspace( -60, 0, 4 ))
dictPrint(it)
sep()
print('i testName [tuneZ1, tuneX1,tuneZ2, tuneX2]')

for tuneX1 in it['tuneX1']:
    for tuneZ1 in it['tuneZ1']:
        for tuneX2 in it['tuneX2']:
            for tuneZ2 in it['tuneZ2']:

                # skip duplicate mirrors
                if tuneX1==tuneX2 and tuneZ1==tuneZ2: continue

                # set test name
                testName = 'focus'
                varList = [tuneX1,tuneZ1,tuneX2,tuneZ2]
                for v in varList: testName += f'__{round(v,1)}'
                print(i,testName,varList)
                outRoot = "out/"+testName+".root"

                # open new compact file
                compactOutName = 'compact/drich.xml'
                compactOutFile = open(athenaDir+'/'+compactOutName,'w')

                # read original compact file, altering specified settings
                compactInFile.seek(0,0)
                for line in compactInFile.readlines():
                    line = re.sub('XXX1',f'{tuneX1}',line)
                    line = re.sub('XXX2',f'{tuneX2}',line)
                    line = re.sub('ZZZ1',f'{tuneZ1}',line)
                    line = re.sub('ZZZ2',f'{tuneZ2}',line)
                    compactOutFile.write(line)
                compactOutFile.close()

                # run npsim, and process results
                def execute(l): subprocess.call(shlex.split(l))
                os.system(f"exit | ./simulate.py -t12 -v -i svg -o {outRoot}")
                #execute(f"./drawHits.exe {outRoot}")

                i+=1

compactInFile.close()
