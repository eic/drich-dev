Float_t sphx,sphy,sphz;
TTree *tr;
TH1D *hist;

// given starting coordinates `start{x,y,z}`, scan through a 
// lattice of points, with size `size`; for each lattice point,
// compute the RMS of the distance from that point to all the 
// photosensors; the closer this lattice point is to the spherical
// center, the smaller the RMS will be; coordinates `end{x,y,z}` will
// be the best lattice point, `enddist` will be the mean distance
// too the photosensor points, and `endrms` will be the rms
void FindCenter(Float_t startx, Float_t starty, Float_t startz,
  Float_t size,
  Float_t &endx, Float_t &endy, Float_t &endz,
  Float_t &enddist, Float_t &endrms) {

  endrms = 1e6;
  enddist = 1e6;

  Float_t currx,curry,currz;
  Float_t currrms;

  int ndiv = 5; // number of lattice points in 1 dimension; keep it odd
  Float_t div = size/ndiv; // lattice step size
  int rc = (ndiv-1)/2;

  // lattice loop
  for(int x=0; x<ndiv; x++) {
  for(int y=0; y<ndiv; y++) {
  for(int z=0; z<ndiv; z++) {

    // current lattice point
    currx = startx + (x-rc)*div;
    curry = starty + (y-rc)*div;
    currz = startz + (z-rc)*div;
    //printf("%f %f %f\n",currx,curry,currz);

    // calculate RMS of distances between this
    // lattice point and the photosensors
    hist->Reset();
    for(int e=0; e<tr->GetEntries(); e++) {
      tr->GetEntry(e);
      hist->Fill(
          TMath::Sqrt(
            TMath::Power(sphx-currx,2)+
            TMath::Power(sphy-curry,2)+
            TMath::Power(sphz-currz,2)
            )
          );
    };
    currrms = hist->GetStdDev();
    //printf("   rms = %f\n",currrms);

    // if this is the best RMS, set result values
    if(currrms<endrms) {
      endx = currx;
      endy = curry;
      endz = currz;
      endrms = currrms;
      enddist = hist->GetMean();
    };

  };};}; // end lattice loop

  // print result
  printf("result: (%.2f, %.2f, %.2f)  rms=%.4f  dist=%.4f\n",
    endx,endy,endz,endrms,enddist);
};

//////////////////////////////////////////////////

void FindSphereCenter(TString treeFile="photosensors.dat") {

  // read photosensor cooridnates
  tr = new TTree();
  tr->ReadFile(treeFile,"x/F:y/F:z/F");
  tr->SetBranchAddress("x",&sphx);
  tr->SetBranchAddress("y",&sphy);
  tr->SetBranchAddress("z",&sphz);

  // histogram for calculating RMS
  hist = new TH1D("hist","hist",2000,0,2000);

  Float_t guessx,guessy,guessz;
  Float_t bestx,besty,bestz;
  Float_t size,dist,rms;

  // hyperparameters
  const int NIter = 1000; // number of iterations
  Float_t initsize = 300; // initial lattice size
  guessx = 0; // initial guess
  guessy = 0;
  guessz = 0;

  // iteratively search for the center: start with
  // initial guess and lattice size, and get the best lattice point;
  // then start with this best lattice point, and search again with
  // a lattice half the size; since each subsequent lattice is smaller
  // and the number of lattice points is constant, each lattice will
  // be more dense, and hopefully we will converge on the true sphere
  // center and radius
  for(int d=0; d<NIter; d++) {
    size = d>0 ? initsize/(2*d) : initsize;
    FindCenter(
      guessx,guessy,guessz,
      size,
      bestx,besty,bestz,
      dist,rms
      );
    guessx = bestx;
    guessy = besty;
    guessz = bestz;
  };
};
