#pragma once
#include "graph.h"

struct PathResult {
    vector<double> dist;   // min travel time to every node
    vector<int> prev;      // predecessor for path reconstruction
};

PathResult dijkstra(const Graph& g, int src);
vector<int> buildPath(const PathResult& r, int src, int dst);