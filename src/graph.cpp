#include "graph.h"
#include <fstream>
#include <iostream>

void Graph::addNode(const Node& n) {
    nodes.push_back(n);
    adj.push_back({});   //by default empty since no connections are made 
}

void Graph::addEdge(int u, int v, double len, double spd, bool oneWay) {
    adj[u].push_back(Edge(v, len, spd)); //connect that particular node u via another edge(vertex num, length, speed)
    if (!oneWay) adj[v].push_back(Edge(u, len, spd)); //if not one way create an edge in the opp direction too
}

bool Graph::load(const string& file) {
    ifstream in(file);
    if (!in) { 
        cerr << "Cannot open " << file <<endl;  //char error 
        return false;
    }
    int n, m; //input nodes and edges 
    in >> n >> m;
    for (int i = 0; i < n; i++) { //reads the next 8 lines after n,m 
        int id;
        double x, y; //coordinates of node 
        string name; //name of the node 
        in >> id >> x >> y >> name;
        addNode(Node(id, x, y, name)); //pushes back the nodes into the list 
    }
    for (int i = 0; i < m; i++) {
        int u, v, oneway; //to, fro, direction 
        double len, spd; //length, speed 
        in >> u >> v >> len >> spd >> oneway;
        addEdge(u, v, len, spd, oneway == 1);
        // checking if actual road distance is shorter than straight-line distance (which should not happen), allowing a tiny tolerance for decimal precision errors
        if (len < straightKm(u, v) - 1e-9)  
            cerr << "Warning: edge " << u << "-" << v << " is shorter than the straight line\n";
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
    double dx = nodes[a].getX() - nodes[b].getX(); //horizontal distance between two nodes
    double dy = nodes[a].getY() - nodes[b].getY(); //vertical distance between two nodes 
    return sqrt(dx*dx + dy*dy) / PX_PER_KM; //distance formula is being used, dividing it to convert pixels into km
}