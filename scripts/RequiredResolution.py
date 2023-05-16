from math import *
import matplotlib.pyplot as plt
import numpy as np
import getopt
import sys
from matplotlib.ticker import *

narg = len(sys.argv)
if narg == 1:
  print("usage :",sys.argv[0],"1 1");
  print("First number 0 --> pi/K 1--> e/pi");
  print("Second number 0 --> gas 1--> aerogel");
  exit();
agrv1 = sys.argv[1]
agrv2 = sys.argv[2]

file = "";
mass1 = 0;
mass2 = 0;
refIndex =0;
maxmom =0; 

fig, ax = plt.subplots();
ml = AutoMinorLocator(2);
#plt.axes().xaxis.set_minor_locator(ml);
ax.xaxis.set_minor_locator(ml);

if agrv1 == '0':
  mass1 = 0.139;
  mass2 =  0.493; 
  file = "piK.pdf";
  plt.title('pi/K separation');
  if agrv2 == '0':
    refIndex = 1.0008;
    maxmom = 60;
    plt.ylim([0.1,20]);
  elif agrv2 == '1':
    refIndex = 1.02;
    maxmom = 20;
    plt.ylim([0.1,30]);

elif agrv1 == '1':
  mass1 = 0.000511;
  mass2 =  0.139;
  file = "epi.pdf";
  plt.title("e-Pi Separation");
  if agrv2 == '0':
    refIndex = 1.0008;
    maxmom = 20;
    plt.ylim([0.1,100]);
  elif agrv2 == '1':
    refIndex = 1.02;
    maxmom =10;
    plt.ylim([0.1,100]);
  
p=[0.0];
y=[0.0];
z=[0.0];
m=[0.0];
#refIndex = 1.0008;
#mass1=[0.139,0.000511];
#mass2=[0.493,0.139];

print(mass1,mass2,refIndex,maxmom);

def computebeta(zz,mm):
  return zz/sqrt(zz*zz+mm*mm);

def computetheta(beta):
  csth = (1/(refIndex*beta));
  if(csth>1) : return 0;
  else : return acos(csth);

for x in range(0,maxmom,1):
  p.append(x);
  y.append(1000*computetheta(computebeta((x+0.1),mass1)));
  z.append(1000*computetheta(computebeta((x+0.1),mass2)));

Y = np.array(y);
Z = np.array(z);
m_a =np.subtract(Y,Z);
m_0 = np.divide(m_a,3);
m_1 = np.divide(m_a,3.25);
m_2 = np.divide(m_a,3.50);
m_3 = np.divide(m_a,3.75);
m_4 = np.divide(m_a,4);
m = list(m_0);
#print(p);
#print(y);  
#plt.plot(p, y, color='green', linestyle='solid', linewidth = 1.5,
#         marker='o', markerfacecolor='blue', markersize=0.5);
plt.plot(p, m, color='magenta', linestyle='solid', linewidth = 1.5,
         marker='o', markerfacecolor='blue', markersize=1.0, label="3");
del m;
m = list(m_1);
plt.plot(p, m, color='purple', linestyle='solid', linewidth = 1.5,
         marker='o', markerfacecolor='blue', markersize=1.0,label="3.25");
del m;
m = list(m_3);
plt.plot(p, m, color='blue', linestyle='solid', linewidth = 1.5,
         marker='o', markerfacecolor='blue', markersize=1.0,label="3.5");
del m;
m = list(m_3);
plt.plot(p, m, color='cyan', linestyle='solid', linewidth = 1.5,
         marker='o', markerfacecolor='blue', markersize=1.0,label="3.75");
del m;
m = list(m_4);
plt.plot(p, m, color='green', linestyle='solid', linewidth = 1.5,
         marker='o', markerfacecolor='blue', markersize=1.0,label="4");


plt.yscale('log');
plt.xlabel('momentum (GeV/c)');
plt.ylabel('required resolution (mrad)');
plt.legend();
plt.grid(True);
plt.grid(which='minor', alpha=0.3) 
#plt.show();
#fig, ax = plt.subplots();
#ml = AutoMinorLocator(2);
#plt.axes().xaxis.set_minor_locator(ml);
#ax.xaxis.set_minor_locator(ml);
plt.savefig(file);

