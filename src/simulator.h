#pragma once
#include <vector>
#include <string>
#include "graph.h"
#include "dijkstra.h"
#include "ambulance.h"
#include "incident.h"
#include "dispatcher.h"
#include "scenario.h"
#include "metrics.h"
using namespace std;

const double SCENE_TIME = 5.0;       // minutes an ambulance stays with the patient

// What one ambulance is currently doing (one incident at a time).
struct Mission {
    bool active = false;
    Incident inc;                    // the incident being served
    int target = -1;                 // node it is heading to now (incident, then hospital)
    vector<int> plan;                // planned route, plan[0] = the node it is at / left from
    int from = -1, to = -1;          // road being driven (to == -1: standing at an intersection)
    double edgeTotal = 0;            // minutes the current road takes
    double edgeLeft = 0;             // minutes still to go on it
    double sceneLeft = 0;            // minutes left at the scene
    bool waiting = false;            // no open route right now

    // 0..1 progress along the current road (the GUI uses this to place the ambulance)
    double progress() const {
        return (to == -1 || edgeTotal <= 0) ? 0.0 : 1.0 - edgeLeft / edgeTotal;
    }
};

class Simulator {
    Graph g;                         // private copy: closures and congestion change it
    Scenario sc;
    Strategy strategy_;
    bool verbose_;                   // print the decision log as it happens

    vector<Ambulance> fleet_;
    vector<Mission> missions_;       // missions_[i] belongs to fleet_[i]
    IncidentQueue queue_;
    Dispatcher dispatcher_;
    Metrics metrics_;
    vector<string> log_;

    double now_ = 0;
    size_t nextEvent_ = 0;
    int nextIncidentId_ = 1;
    int stalledIncident_ = -1;       // incident we already reported as unreachable

    // helpers
    const string& nm(int node) const { return g.getNode(node).getName(); }
    string routeStr(const vector<int>& path) const;
    bool roadBlocked(int u, int v) const;
    void note(double t, const string& msg);

    // the simulation steps
    void processEvents();
    void dispatchPending();
    void advanceAmbulance(int idx, double dt);
    void onArrive(int idx, double clock);
    bool decideNext(int idx, double clock);       // the decision made at every intersection
    bool startHospitalLeg(int idx, double clock);
    void finishMission(int idx, double clock);

public:
    Simulator(Graph graph, const Scenario& scenario, Strategy s, bool verbose);

    void step(double dt);                         // advance the clock by dt minutes
    void run(double dt = 0.1, double maxTime = 600.0);
    bool finished() const;

    // read-only access for printing now and for the GUI later
    double now() const { return now_; }
    Strategy strategy() const { return strategy_; }
    const Graph& graph() const { return g; }
    const vector<Ambulance>& fleet() const { return fleet_; }
    const Mission& mission(int i) const { return missions_[i]; }
    const Metrics& metrics() const { return metrics_; }
    const vector<string>& log() const { return log_; }
};
