#include "dijkstra.h"
#include <queue>
#include <algorithm>
#include<heap.h>

PathResult dijkstra(const Graph& g, int src, bool live) {
    int n = g.size();
    PathResult r;
    r.dist.assign(n, INF);
    r.prev.assign(n, -1);
    using P = pair<double,int>;
    Heap<P, greater<P>> pq;
    r.dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > r.dist[u]) continue;                  // stale entry
        for (const Edge& e : g.neighbours(u)) {
            if (e.isBlocked()) continue;              // closed road
            int v = e.getTo();
            double w = live ? e.travelTime() : e.baseTravelTime();
            double nd = d + w;
            if (nd < r.dist[v]) {
                r.dist[v] = nd;
                r.prev[v] = u;
                pq.push({nd, v});
            }
        }
    }
    return r;
}

vector<int> buildPath(const PathResult& r, int src, int dst) {
    vector<int> path;
    if (r.dist[dst] >= INF) return path;              // unreachable
    for (int v = dst; v != -1; v = r.prev[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}
