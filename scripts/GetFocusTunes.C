void GetFocusTunes(){
    printf("Mode: (sensor centre adjustement 1 and radius mag. 2)\n");
    int mode;
    //cin>>mode;
    scanf("%d",&mode);
    double zs,xs;
    double xs_nom = 1834; //mm
    double zs_nom = 1384; //mm
    double Rs_nom = 1100; //mm

    double xm_nom = 114.582; //cm
    double zm_nom = 288.894; //cm
    double dRICH_Zmax = 315.; //cm
    double dRICH_Zmin = 195.; //cm
    double back = 1.1; // 0.1 cm of thickness  + 1 cm  space  from the end of the dRICH

    if(mode == 1){  
      double DL; 
      printf("How much shift?(mm) \n");
      scanf("%lf",&DL);
      zs = zs_nom + DL * cos(22/180.*3.1415) ; //mm
      xs = xs_nom - DL * sin(22/180.*3.1415) ; //mm
    
      zs/=10.;
      xs/=10.;
      printf("xs,ys,zs,Rs:%0.3lf 0 %0.3lf 110 \n",xs,zs);
      printf("xm,ym,zm,Rm:%0.3lf 0 %0.3lf 220 \n",xm_nom,zm_nom);
    }
    if(mode == 2){
      cout<<"How much mag.? "<<endl;
      int X;
      cin>>X;
      zs = zs_nom - (X-1) * Rs_nom* cos(22/180.*3.1415); // mm
      xs = xs_nom + (X-1) * Rs_nom* sin(22/180.*3.1415); // mm 
      zs/=10.;
      xs/=10.;
      printf("xs,ys,zs,R:%0.3lf 0 %0.3lf %0.3lf \n",xs,zs,X*110.);
      printf("xm,ym,zm,Rm:%0.3lf 0 %0.3lf 220 \n",xm_nom,zm_nom);
    }

		double b = dRICH_Zmax - back;
		double zf = (2*b*(zm_nom-dRICH_Zmin))/(b+(zm_nom-dRICH_Zmin)); 
		double xf = (xm_nom/b)*(2*b - ((2*b*(zm_nom-dRICH_Zmin))/(b+(zm_nom-dRICH_Zmin)))); 


		double fz = (zf - zs);
		double fx = (xf - xs);


		printf("focus tunes: fx fz %0.2lf %0.2lf\n",fx,fz);


}
