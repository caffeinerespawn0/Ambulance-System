#include <iostream>
#include "graph.h"
#include "dispatcher.h"

int main() {
    Graph g;
    g.load("data/map.txt");
    g.setCongestion(6, 7, 3.0);                 // Station_2 -> Incident now 3x slower

    vector<Ambulance> fleet = { 
        Ambulance(1, 3), Ambulance(2, 6) //1st ambulance: node 3, 2nd ambulance node 6
    };  // Station_1, Station_2
    
    Incident inc(1, 7, 5, 0);                   //read as incident 1, node 7, sev 5, arrivaltime 0

    Dispatcher d;
    const char* names[] = {"NEAREST_STRAIGHT", "STATIC_SHORTEST", "DYNAMIC"};
    Strategy all[] = {Strategy::NEAREST_STRAIGHT, Strategy::STATIC_SHORTEST, Strategy::DYNAMIC};

    for (int i = 0; i < 3; i++) {
        DispatchResult r = d.choose(g, fleet, inc, all[i]);
        cout << names[i] << ": ambulance " << r.ambulanceId << ", ETA " << r.eta << " min, path:";
        for (int v : r.path) cout << " " << g.getNode(v).getName();
        cout <<endl;
    }
} 