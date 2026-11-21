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
        sim.addAmbulance(3);
        sim.addAmbulance(6);
        sim.addEvent(0,SET_TRAFFIC,6,7,3.0);
        sim.addEvent(0,NEW_INCIDENT,7,5,0);
        sim.addEvent(1,NEW_INCIDENT,5,3,0);
        cout<<"--- strategy: "<<names[s]<<" ---"<<endl;
        sim.processEvents(0);
        sim.assignAmbulances();
        sim.processEvents(1);
        sim.assignAmbulances();
    }
    cout<<"--- priority: one ambulance, three incidents at t=0 ---"<<endl;
    Simulation sim(g,DYNAMIC);
    sim.addAmbulance(3);
    sim.addEvent(0,NEW_INCIDENT,7,2,0);
    sim.addEvent(0,NEW_INCIDENT,2,5,0);
    sim.addEvent(0,NEW_INCIDENT,5,3,0);
    sim.processEvents(0);
    sim.assignAmbulances();
    return 0;
}
