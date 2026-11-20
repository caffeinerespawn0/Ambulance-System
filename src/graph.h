#pragma once 
#include <vector>
#include<string>
#include<cmath> 
using namespace std; 
const double PX_PER_KM=75.0; 
class Node{
    int id; 
    double x,y; //coordinates (to plot map)
    string name; //name of the node 
    public: 
    Node():id(-1),x(0),y(0){}; 
    Node(int id, double x, double y, const string& name):id(id),x(x),y(y),name(name){}; 
    //getter funcs 
    int getID() const{
        return id; 
    }
    double getX() const{
        return x; 
    }
    double getY() const{
        return y; 
    }
    const string& getName() const{
        return name; 
    }
};
class Edge{
    int to; 
    double length; //standard km 
    double speed;  //standard km/h
    double congestion; //1.0=free flow, 2.0=slow 
    bool blocked;  
    public: 
    //by default: free flow 
    Edge(int to,double length, double speed):to(to),length(length),speed(speed),congestion(1.0),blocked(false){}; 
    //getter funcs 
    double getLength() const{
        return length; 
    }
    double getSpeed() const{
        return speed; 
    }
    double getCongestion()const{
        return congestion; 
    }
    bool isBlocked()const{
        return blocked; 
    }
    void setBlocked(bool b){
        blocked=b; 
    }
    void setCongestion(double c){
        congestion=c; 
    }
    double travelTime()const{
        //formula we're using throughout: length/speed*60*congestion (dijkstras weight)
        //result: minutes (dijkstra's weight)
        return (length/speed)*(60.0)*congestion; 
    }
    double baseTravelTime()const{   //w/out congestion 
        return (length/speed)*(60.0); 
    }
    int getTo() const{
        return to; 
    }
}; 
class Graph{
    vector<Node> nodes; 
    vector<vector<Edge>> adj; 
    public: 
    bool load(const string& file); 
    void addNode(const Node& n); 
    void addEdge(int u, int v, double len, double spd, bool oneWay); 
    int size() const{
        return nodes.size(); 
    }
    const Node& getNode(int id) const {
        return nodes[id]; 
    }
    const vector<Edge>& neighbours(int u)const{
        return adj[u]; //returning the neighbours of u (all the neighbours of the node u)
    }
    bool setBlocked(int u, int v, bool state, bool bothWays=false); 
    bool setCongestion(int u, int v, double factor); 
    double straightKm(int a, int b) const;
}; 