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
const double SCENE_TIME=5.0; //minutes the ambulance stays with the patient
const double TICK=0.1; //minutes the clock moves in one step
struct Ambulance{
    int id;
    int node; //last intersection it was at
    int state;
    int incident; //index of the incident it is serving, -1 if none
    int target; //node it is heading to (the incident, then a hospital)
    vector<int> route; //planned route, route[0] is the node it is at
    int nextNode; //end of the road it is driving on, -1 if standing at an intersection
    double timeLeft; //minutes left on the current road
    double roadTime; //minutes the current road takes in total
    double sceneLeft; //minutes left at the scene
    bool stuck; //true while there is no open route
};
struct Incident{
    int node;
    int severity; //1 (minor) to 5 (critical)
    double arrivalTime;
    double dispatchTime; //-1 until an ambulance is sent
    double reachTime; //-1 until an ambulance arrives
    double deliverTime; //-1 until the patient is at a hospital
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
    vector<int> hospitals;
    Heap waiting; //max heap of waiting incidents, id is the index in incidents
    int nextEvent;
    int ticks;
    double now;
    int reroutes;
    long long algoNanos; //time spent inside dijkstra
    void showDistances(int index);
    int chooseAmbulance(int index, double& eta);
    void arrive(int a);
    void decideNextRoad(int a);
    void goToHospital(int a);
    void moveAmbulances();
    double routeTime(const vector<int>& route);
    void runDijkstra(int src, vector<double>& dist, vector<int>& prev, bool useTraffic);
    public:
    Simulation(const Graph& map, int s);
    void addAmbulance(int node);
    void addHospital(int node);
    void addEvent(double time, int type, int a, int b, double value);
    void processEvents(double upTo);
    void assignAmbulances();
    void step();
    bool finished();
    void run();
    void printResults();
    double avgResponse(int minSeverity);
    double maxResponse();
    double algoMicros();
    //read only access for the gui
    const Graph& getGraph() const{
        return g;
    }
    double getNow() const{
        return now;
    }
    int getReroutes() const{
        return reroutes;
    }
    int ambulanceCount() const{
        return ambulances.size();
    }
    const Ambulance& getAmbulance(int i) const{
        return ambulances[i];
    }
    int incidentCount() const{
        return incidents.size();
    }
    const Incident& getIncident(int i) const{
        return incidents[i];
    }
    const vector<int>& getHospitals() const{
        return hospitals;
    }
};
//fills a simulation with one of the test scenarios (1 to 5)
void loadScenario(Simulation& sim, int number);
 
