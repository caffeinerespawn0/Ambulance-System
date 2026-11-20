#pragma once
#include <vector>
#include "graph.h"
#include "dijkstra.h"
#include "ambulance.h"
#include "incident.h"
using namespace std;

enum class Strategy {
    NEAREST_STRAIGHT,    // smallest straight-line distance (ignores roads and traffic)
    STATIC_SHORTEST,     // Dijkstra with fixed travel times (ignores congestion)
    DYNAMIC              // Dijkstra with live travel times (the proposed method)
};

struct DispatchResult {
    int ambulanceId = -1;        // -1 means nobody could be sent
    double eta = INF;            // live minutes to reach the incident
    vector<int> path;            // live route as node ids
    bool found() const { return ambulanceId != -1; }
};

// One row of the "overall distances" table shown when an incident is dispatched.
struct Candidate {
    int ambulanceId;
    double straightKm;           // straight-line distance
    double staticMin;            // road travel time with fixed weights
    double liveMin;              // road travel time with current traffic (INF = unreachable)
};

class Dispatcher {
public:
    DispatchResult choose(const Graph& g, const vector<Ambulance>& fleet,
                          const Incident& inc, Strategy s) const {
        int target = inc.getNode();
        int best = -1;                  // index of the best ambulance so far
        double bestScore = INF;         // lower score = better

        // 1. Score every idle ambulance by the chosen strategy.
        for (int i = 0; i < (int)fleet.size(); i++) {
            if (!fleet[i].isIdle()) continue;               // busy: skip

            double score;
            if (s == Strategy::NEAREST_STRAIGHT) {
                score = g.straightKm(fleet[i].getNode(), target);
            } else {
                // live = true only for DYNAMIC; STATIC_SHORTEST ignores congestion
                PathResult r = dijkstra(g, fleet[i].getNode(), s == Strategy::DYNAMIC);
                score = r.dist[target];
            }
            if (score < bestScore) { bestScore = score; best = i; }
        }

        DispatchResult res;
        if (best == -1) return res;                         // nobody available

        // 2. Report what the chosen ambulance REALLY faces: always the live route.
        int from = fleet[best].getNode();
        PathResult live = dijkstra(g, from);                // live = true by default
        if (live.dist[target] >= INF) return res;           // unreachable right now
        res.ambulanceId = fleet[best].getId();
        res.eta = live.dist[target];
        res.path = buildPath(live, from, target);
        return res;
    }

    // Distances from every idle ambulance to the incident, for display only.
    vector<Candidate> candidates(const Graph& g, const vector<Ambulance>& fleet,
                                 const Incident& inc) const {
        vector<Candidate> out;
        int target = inc.getNode();
        for (const Ambulance& a : fleet) {
            if (!a.isIdle()) continue;
            Candidate c;
            c.ambulanceId = a.getId();
            c.straightKm = g.straightKm(a.getNode(), target);
            c.staticMin = dijkstra(g, a.getNode(), false).dist[target];
            c.liveMin   = dijkstra(g, a.getNode(), true).dist[target];
            out.push_back(c);
        }
        return out;
    }
};