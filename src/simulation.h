#pragma once
#include <vector>
#include <string>
#include "graph.h"
#include "heap.h"
using namespace std;
//ambulance states
const int IDLE=0;
const int TO_INCIDENT=1;
const int AT_SCENE=2;
const int TO_HOSPITAL=3;
//dispatch strategies
const int NEAREST=1; //straight line distance only
const int STATIC_ROUTE=2; //road time with fixed weights (no traffic)
const int DYNAMIC=3; //road time with current traffic
//event types
const int NEW_INCIDENT=1; //a=node, b=severity
const int CLOSE_ROAD=2; //a to b
const int OPEN_ROAD=3; //a to b
const int SET_TRAFFIC=4; //a to b, value=congestion factor
const double SEVERITY_WEIGHT=10.0;
struct Ambulance{
    int id;
    int node; //where it is now
    int state;
    int incident; //index of the incident it is serving, -1 if none
};
struct Incident{
    int node;
    int severity; //1 (minor) to 5 (critical)
    double arrivalTime;
    double dispatchTime; //-1 until an ambulance is sent
    int ambulanceId; //-1 until an ambulance is sent
};
struct Event{
    double time;
    int type;
    int a;
    int b;
    double value;
};
class Simulation{
    Graph g; //own copy of the map, roads change during the run
    int strategy;
    vector<Ambulance> ambulances;
    vector<Incident> incidents;
    vector<Event> events; //added in time order
    Heap waiting; //max heap of waiting incidents, id is the index in incidents
    int nextEvent;
    double now;
    void showDistances(int index);
    int chooseAmbulance(int index, double& eta);
    public:
    Simulation(const Graph& map, int s);
    void addAmbulance(int node);
    void addEvent(double time, int type, int a, int b, double value);
    void processEvents(double upTo);
    void assignAmbulances();
};
