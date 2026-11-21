#include "dijkstra.h"
#include <algorithm>
#include "heap.h"

PathResult dijkstra(const Graph& g, int src, bool live) {
    int n = g.size();
    PathResult r;
    r.dist.assign(n, INF); //all the n nodes are assigned with infinity as the distance (which are replaced as we search further)
    r.prev.assign(n, -1);
    Heap pq(true);
    r.dist[src] = 0; //source is assigned a distance of 0 
    pq.push(src, 0);

    while (!pq.isEmpty()) {
        HeapItem cur = pq.pop();
        int u = cur.id;
        double d = cur.priority;
        if (d > r.dist[u])
            continue;                  // stale entry
        for (const Edge& e : g.neighbours(u)) {
            if (e.isBlocked()){
                 continue;              // closed road
            }
            int v=e.getTo(); 
            double w; 
            if(live==true){
                w=e.travelTime(); 
            }
            else{
                w=e.baseTravelTime(); 
            }
            double nd=d+w; 
            if (nd < r.dist[v]) {
                r.dist[v] = nd;
                r.prev[v] = u;
                pq.push(v, nd);
            }
        }
    }
    return r;
}

vector<int> buildPath(const PathResult& r, int src, int dst) {
    vector<int> path;
    if (r.dist[dst] >= INF){        //If it is still INF Dijkstra couldn't find a route.
         return path;              // unreachable
    }
    if (dst == src) {
        path.push_back(src);
        return path;
    }
    for (int v = dst; v != -1; v = r.prev[v]){
        path.push_back(v);
    }
    reverse(path.begin(), path.end());
    return path;
}
