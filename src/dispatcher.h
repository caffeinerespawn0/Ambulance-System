#pragma once
#include <vector>
#include "graph.h"
#include "dijkstra.h"
#include "ambulance.h"
#include "incident.h"
using namespace std;

enum class Strategy {
    NEAREST_STRAIGHT, STATIC_SHORTEST, DYNAMIC
};

struct DispatchResult {
    int ambulanceId = -1;
    double eta = INF;        // live minutes to reach the incident
    vector<int> path;        // live route, as node ids
    bool found() const { return ambulanceId != -1; }
};

class Dispatcher {
public:
    DispatchResult choose(const Graph& g, const vector<Ambulance>& fleet,
                          const Incident& inc, Strategy s) const {
        int target = inc.getNode();
        int best = -1;
        double bestScore = INF;

        for (int i = 0; i < (int)fleet.size(); i++) {
            if (!fleet[i].isIdle()) continue;
            double score;
            if (s == Strategy::NEAREST_STRAIGHT)
                score = g.straightKm(fleet[i].getNode(), target);
            else {
                PathResult r = dijkstra(g, fleet[i].getNode(), s == Strategy::DYNAMIC);
                score = r.dist[target];
            }
            if (score < bestScore) { bestScore = score; best = i; }
        }

        DispatchResult res;
        if (best == -1){ 
        return res;                       // nobody available
        }
        int from = fleet[best].getNode();
        PathResult live = dijkstra(g, from);              // real conditions
        if (live.dist[target] >= INF){
            return res;         // unreachable
        }
        res.ambulanceId = fleet[best].getId();
        res.eta = live.dist[target];
        res.path = buildPath(live, from, target);
        return res;
    }
};