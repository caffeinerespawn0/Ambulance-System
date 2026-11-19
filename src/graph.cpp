#include "graph.h"
#include <fstream>
#include <iostream>

void Graph::addNode(const Node& n) {
    nodes.push_back(n);
    adj.push_back({});
}

void Graph::addEdge(int u, int v, double len, double spd, bool oneWay) {
    adj[u].push_back(Edge(v, len, spd));
    if (!oneWay) adj[v].push_back(Edge(u, len, spd));
}

bool Graph::load(const string& file) {
    ifstream in(file);
    if (!in) { cerr << "Cannot open " << file << "\n"; return false; }
    int n, m;
    in >> n >> m;
    for (int i = 0; i < n; i++) {
        int id; double x, y; string name;
        in >> id >> x >> y >> name;
        addNode(Node(id, x, y, name));
    }
    for (int i = 0; i < m; i++) {
        int u, v, oneway; double len, spd;
        in >> u >> v >> len >> spd >> oneway;
        addEdge(u, v, len, spd, oneway == 1);
        if (len < straightKm(u, v) - 1e-9)
            cerr << "Warning: edge " << u << "-" << v
                 << " is shorter than the straight line\n";
    }
    return true;
}

bool Graph::setBlocked(int u, int v, bool state, bool bothWays) {
    bool found = false;
    for (Edge& e : adj[u])
        if (e.getTo() == v) { e.setBlocked(state); found = true; }
    if (bothWays)
        for (Edge& e : adj[v])
            if (e.getTo() == u) e.setBlocked(state);
    return found;
}

bool Graph::setCongestion(int u, int v, double factor) {
    for (Edge& e : adj[u])
        if (e.getTo() == v) { e.setCongestion(factor); return true; }
    return false;
}

double Graph::straightKm(int a, int b) const {
    double dx = nodes[a].getX() - nodes[b].getX();
    double dy = nodes[a].getY() - nodes[b].getY();
    return sqrt(dx*dx + dy*dy) / PX_PER_KM;
}