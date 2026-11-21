#pragma once
#include "graph.h"

struct PathResult {
    vector<double> dist; //stores shortest known distance 
    vector<int> prev;   
};

PathResult dijkstra(const Graph& g, int src, bool live = true);
vector<int> buildPath(const PathResult& r, int src, int dst);