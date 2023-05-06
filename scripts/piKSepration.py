from math import *
import matplotlib.pyplot as plt
import numpy as np

p=[0.0];
y=[0.0];
z=[0.0];
m=[0.0];
refIndex = 1.0008;

def computebeta(zz,mm):
  return zz/sqrt(zz*zz+mm*mm);

def computetheta(beta):
  csth = (1/(refIndex*beta));
  if(csth>1) : return 0;
  else : return acos(csth);

for x in range(0,60,1):
  p.append(x);
  y.append(1000*computetheta(computebeta((x+0.1),0.139)));
  z.append(1000*computetheta(computebeta((x+0.1),0.493)));

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
plt.ylim([0.1,20]);
plt.xlabel('momentum (GeV/c)');
plt.ylabel('required resolution (mrad)');
plt.title('pi/K separation');
plt.legend();
plt.grid(True);
plt.grid(which='minor', alpha=0.3) 
#plt.show();
plt.savefig("piK.pdf");

