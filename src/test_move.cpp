#include <iostream>
#include "simulation.h"
using namespace std;
int main(){
    Graph g;
    if(!g.load("data/map.txt")){
        return 1;
    }
    string names[4]={"","nearest","static route","dynamic"};
    for(int s=1;s<=3;s++){
        Simulation sim(g,s);
        sim.addAmbulance(0);
        sim.addHospital(0);
        sim.addEvent(0,NEW_INCIDENT,7,5,0);
        sim.addEvent(1,SET_TRAFFIC,6,7,3.0);
        cout<<"--- strategy: "<<names[s]<<" ---"<<endl;
        sim.run();
    }
    return 0;
}

