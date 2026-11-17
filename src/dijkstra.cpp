#include "dijkstra.h"
#include <queue>
#include <algorithm>
const double INF = 1e18;

PathResult dijkstra(const Graph& g, int src) {
    int n = g.size();
    PathResult r; r.dist.assign(n, INF); r.prev.assign(n, -1);
    using P = pair<double,int>;                       // (distance, node)
    priority_queue<P, vector<P>, greater<P>> pq;      // min-heap
    r.dist[src] = 0; pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > r.dist[u]) continue;                  // stale entry
        for (const Edge& e : g.neighbors(u)) {
            if (e.isBlocked()) continue;              // closed road
            int v = e.getTo();
            double nd = d + e.travelTime();
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