void reader_2(const char *dfname, const char *cfname, const char *dtname = 0)
{
#define _AEROGEL_
#define _DRICH_

// Optionally: mimic low wave length cutoff and average QE x Geometric sensor efficiency;
#define _WAVE_LENGTH_CUTOFF_MIN_ (350.0)
#define _WAVE_LENGTH_CUTOFF_MAX_ (650.0)
  //#define _AVERAGE_PDE_            ( 0.30)

  // .root file with event tree;
  auto fdata = new TFile(dfname);
  TTree *t = (TTree*)fdata->Get("events");
  //t->SetMakeClass(1);
  //t->SetBranchStatus("*", 0);
  // .root file with the IRT configuration;
  auto fcfg  = new TFile(cfname);
  CherenkovDetector *detector = 0;
  auto geometry = dynamic_cast<CherenkovDetectorCollection*>(fcfg->Get("CherenkovDetectorCollection"));

  if (dtname) 
    detector = geometry->GetDetector(dtname);
  else {
    // Assume a single detector (PFRICH or DRICH);
    auto &detectors = geometry->GetDetectors();
    if (detectors.size() != 1) {
      printf("More than one detector in the provided IRT geometry config .root file!\n");
      exit(0);
    } //if

    detector = (*detectors.begin()).second;
  } //if

  // Either this or that way, the detector should be available;
  if (!detector) {
    printf("Was not able to find a valid Cherenkov detector in the provided IRT geometry config .root file!\n");
    exit(0);
  } //if

  auto ed = new TH2D("ed","ed;x[cm];y[cm]",1000,-200,200, 1000,-200,200);
  auto nq = new TH1D("nq", "Photon count",            50,      0,    100);
  auto np = new TH1D("np", "Photon count",            50,      0,    100);
  //auto fi = new TH1D("fi", "Cherenkov phi angle",      30,-180.0,  180.0);
  //auto xi = new TH1D("xi", "Chi square CDFC (photon)",100,   0.0,    1.0);
  //auto wi = new TH1D("wi", "Chi square CDFC (track)",  20,   0.0,    1.0);
  //fi->SetMinimum(0); xi->SetMinimum(0); wi->SetMinimum(0);
#ifdef _AEROGEL_
  //auto pl = new TH1D("pl", "Radiator path Length",    100,   0.0,   50.0);
  auto ep = new TH1D("ep", "Emission Point",          100, -30.0,   30.0);
  //auto ri = new TH1D("ri", "1.0 - Refractive Index",  100, 0.015,  0.025);
#else
  //auto pl = new TH1D("pl", "Radiator path Length",    100,   0.0, 2000.0);
  //auto ep = new TH1D("ep", "Emission Point",          100, -70.0,   70.0);
  //auto ri = new TH1D("ri", "1.0 - Refractive Index",  100, 0.015,  0.025);
#endif
  auto wl = new TH1D("wl", "Wave Length; #{Lambda} (nm)",             100, 200.0, 1000.0);
  //auto th = new TH1D("th", "Cherenkov angle",         100, -10,  10);
  //auto tq = new TH1D("tq", "Average Cherenkov angle", 100, -10,  10);
  auto p = new TH1D("p","Mom; p(GeV/c)",50,49.,51.);

  auto gas      = detector->GetRadiator("GasVolume");
  auto aerogel  = detector->GetRadiator("Aerogel");
  //auto acrylic  = detector->GetRadiator("Filter");
  // Assume the reference value was close enough in PFRICH_geo.cpp; since QE was not accounted, 
  // this may not be true; 
  gas    ->m_AverageRefractiveIndex = gas    ->n();
  aerogel->m_AverageRefractiveIndex = 1.020;//aerogel->n();
  //acrylic->m_AverageRefractiveIndex = acrylic->n();

  //#ifdef _DRICH_
  aerogel->SetGaussianSmearing(0.001);
  //#else
  //aerogel->SetUniformSmearing(0.005);
  //#endif
  // Be aware, that AddLocations() part should take this into account;
  aerogel->SetTrajectoryBinCount(2);
  // This may be bogus for a blob-like operation mode;
  //gas    ->SetUniformSmearing(0.003);


  int pdg;
  int q;
  // TTree interface variable;
  auto event = new CherenkovEvent();

  // Use MC truth particles, and deal with just pfRICH hits here; however the interface 
  // should work for combinations like pfRICH+DIRC, eventually; 
  //edm4hep::MCParticleData  *MCParticle;     //= new std::vector<edm4hep::MCParticleData> ();
  //std::vector<edm4hep::SimTrackerHitData> *hits   = new std::vector<edm4hep::SimTrackerHitData>();
  //t->SetBranchAddress("MCParticles",&MCParticle);
  //t->SetBranchStatus("MCParticles.PDG",1);
  TTreeReader myReader("events",fdata);
  TTreeReaderValue <std::vector<edm4hep::MCParticleData>> mcparts(myReader,"MCParticles");
  TTreeReaderValue <std::vector<edm4hep::SimTrackerHitData>> hits(myReader,"DRICHHits");
/* 
  t->SetBranchAddress("MCParticles.PDG", &pdg);
  t->SetBranchAddress("MCParticles.charge", &q);
 // {
   // TString hname; hname.Form("%sHits", detector->GetName());
   // t->SetBranchAddress(hname,   &hits);
 // }
  int myentries = t->GetEntries();
  printf("Entries: %d\n",myentries);
  printf("Here!\n");
  // Loop through all events;
  //unsigned false_assignment_stat = 0;
  for(int ev=0; ev<myentries; ev++) {
    t->GetEntry(ev);
    //int Size = MCParticle->size();
    cout<<"EV: "<<ev<<" Charge:  "<<q<<endl;
    //printf("%d\n", tsize);

  }//ev      
*/
  int evtcounter =0;
  while(myReader.Next()){
    printf("#################\n");
    //evtcounter++;
    //cout<<"MC Size: "<<mcparts->size()<<endl;
    //cout<<"Hit Size: "<<hits->size()<<endl;
    for(auto  && mcpart : *mcparts){
       //cout<<val.size()<<endl;
      if(mcpart.parents_begin!=mcpart.daughters_begin) continue; 
      {
         /*if(mcpart.PDG == 22){ 
           double pmag = TMath::Sqrt(TMath::Power(mcpart.momentum.x,2)+ TMath::Power(mcpart.momentum.y,2) + TMath::Power(mcpart.momentum.z,2));
           cout<<"EV: "<< evtcounter<< " PDG: "<<mcpart.PDG<<" ep x: "<<pmag<<endl;
           p->Fill(pmag);
           double wave_length = 1239.84/(1E9*pmag);
           wl->Fill(wave_length);
           cout<<mcpart.vertex.z<<" "<<mcpart.endpoint.z<<endl;   
         }*/  
         
      }
      int phcounter =0;
      std::vector<OpticalPhoton*> photons;  
      for(auto &&hit:*hits){
        //cout<<"hitsx   "<<hit.momentum.x<<endl;
        double pmag = TMath::Sqrt(TMath::Power(hit.momentum.x,2)+ TMath::Power(hit.momentum.y,2) + TMath::Power(hit.momentum.z,2));
        double wave_length = 1239.84/(1E9*pmag);
        //cout<<"Lambda: "<<wave_length<<endl;
        auto xx = hit.position.x; auto yy = hit.position.y; 
        //ed->Fill(xx/10,yy/10);
        wl->Fill(wave_length); 
        auto photon = new OpticalPhoton();
        {
          auto x = hit.position.x;
          auto y = hit.position.y;
          auto z = hit.position.z;
          printf("Recorded Hit Poistion ------> %f %f %f\n",x,y,z);
          photon->SetDetectionPosition(TVector3(x, y, z));
        }
        // A single photodetector type is used;
        photon->SetPhotonDetector(detector->m_PhotonDetectors[0]);  //?
        photon->SetDetected(true); phcounter+=1;
        // Get cell index; mask out everything apart from {module,sector};
        photon->SetVolumeCopy(hit.cellID & detector->GetReadoutCellMask());
        //photon->SetVolumeCopy(hit.cellID & detector->GetReadoutCellMask());
        photons.push_back(photon);
      }//hit 
      printf("Set True Phot: %d\n",phcounter);
      auto particle = new ChargedParticle(mcpart.PDG);
      event->AddChargedParticle(particle);
      gas->ResetLocations();
      // Create a fake (empty) history; then track locations at the gas boundaries;
      particle->StartRadiatorHistory(std::make_pair(gas, new RadiatorHistory()));
      {
        // FIXME: need it not at vertex, but in the radiator; as coded here, this can
        // hardly work once the magnetic field is turned on;
        auto x0 = TVector3(mcpart.vertex.x, mcpart.vertex.y, mcpart.vertex.z), p0 = TVector3(mcpart.momentum.x, mcpart.momentum.y, mcpart.momentum.z), n0 = p0.Unit();
        printf("Momentum: %0.2f\n",p0.Mag());
        // So, give the algorithm gas surface boundaries as encoded in PFRICH_geo.cpp;
        TVector3 from, to;
        gas->GetFrontSide(0)->GetCrossing(x0, n0, &from);
        gas->GetRearSide (0)->GetCrossing(x0, n0, &to);

        // Move the points a bit inwards;
        TVector3 nn = (to - from).Unit(); from += (0.010)*nn; to -= (0.010)*nn;
        gas->AddLocation(from, p0);
        gas->AddLocation(  to, p0);
        printf("@@@ %f %f\n", from.z(), to.z());// - from.z());
      }
      {
        CherenkovPID pid;

        // Consider just pi/K case for now;
        pid.AddMassHypothesis(0.140);
        pid.AddMassHypothesis(0.494);
        
        printf("Entering PID Rec:%zu\n",photons.size()); 
        particle->PIDReconstruction(pid);
        {
          auto pion = pid.GetHypothesis(0), kaon = pid.GetHypothesis(1);
          double wt0 = pion->GetWeight(gas), wt1 = kaon->GetWeight(gas);

          //th->Fill(pion->GetTheta(gas));

          printf("%10.3f (%10.3f) vs %10.3f (%10.3f) ...  %3d %d\n",
                 wt0, pion->GetNpe(gas), wt1, kaon->GetNpe(gas), particle->GetPDG(), wt0 > wt1);

          //if (wt0 <= wt1) false_assignment_stat++;
        }
      }
      printf("&&&&&&&&&&&&&&\n");  
    }//mctrack       
    cout<<"#################### &&&&&&&&&&&&&   "<< evtcounter   <<"   &&&&&&&&&&&& ######################"<<endl;
    evtcounter++;
  }//ev
  //evtcounter++;
  np->Draw();
} // reader()
