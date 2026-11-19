#pragma once
#include "graph.h"

const double INF = 1e18;

struct PathResult {
    vector<double> dist;
    vector<int> prev;
};

PathResult dijkstra(const Graph& g, int src, bool live = true);
vector<int> buildPath(const PathResult& r, int src, int dst);