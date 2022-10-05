#pragma once
// settings specific to each RICH

#include <string>

class WhichRICH {
  public:

    // detector-specific settings
    bool valid;
    int zDirection;
    std::string xRICH,XRICH;
    std::string sensorNamePattern;
    double plotXmin, plotXmax, plotYmin, plotYmax;
    std::string readoutName;

    // constructor
    WhichRICH(std::string spec) {
      if(spec=="d") {
        valid = true;
        zDirection = 1;
        xRICH = "dRICH";
        XRICH = "DRICH";
        sensorNamePattern = "sensor_de_sec0";
        plotXmin = 100;
        plotXmax = 190;
        plotYmin = -70;
        plotYmax = 70;
      } else if(spec=="p") {
        valid = true;
        zDirection = -1;
        xRICH = "pfRICH";
        XRICH = "PFRICH";
        sensorNamePattern = "sensor_de";
        plotXmin = -70;
        plotXmax = 70;
        plotYmin = -70;
        plotYmax = 70;
      } else {
        valid = false;
        fmt::print(stderr,"ERROR(WhichRICH): unknown argument \"{}\"\n",spec);
      }
      readoutName = XRICH+"Hits";
    }

    ~WhichRICH() {}
};
