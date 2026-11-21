#include <iostream>
#include "graph.h"
using namespace std;
void show(Graph& g, int src, int dst, bool useTraffic, string label){
    vector<double> dist;
    vector<int> prev;
    g.dijkstra(src,dist,prev,useTraffic);
    vector<int> path=g.getPath(prev,src,dst);
    cout<<label<<" eta: ";
    if(path.empty()){
        cout<<"no route"<<endl;
        return;
    }
    cout<<dist[dst]<<" min, path:";
    for(int v: path){
        cout<<" "<<g.getNode(v).getName();
    }
    cout<<endl;
}
int main(){
    Graph g;
    if(!g.load("data/map.txt")){
        return 1;
    }
    show(g,3,7,true,"normal");
    g.setBlocked(6,7,true);
    show(g,3,7,true,"6->7 closed");
    g.setBlocked(6,7,false);
    g.setCongestion(6,7,3.0);
    show(g,6,7,true,"6->7 three times slower, live");
    show(g,6,7,false,"6->7 three times slower, fixed weights");
    g.setBlocked(6,7,true);
    g.setBlocked(5,7,true,true);
    g.setBlocked(4,7,true);
    show(g,3,7,true,"every road into 7 closed");
    return 0;
}
 
