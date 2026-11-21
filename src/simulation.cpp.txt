#include "simulation.h"
#include <iostream>
using namespace std;
Simulation::Simulation(const Graph& map, int s):g(map),strategy(s),waiting(false){
    nextEvent=0;
    now=0;
}
void Simulation::addAmbulance(int node){
    Ambulance a;
    a.id=ambulances.size()+1;
    a.node=node;
    a.state=IDLE;
    a.incident=-1;
    ambulances.push_back(a);
}
void Simulation::addEvent(double time, int type, int a, int b, double value){
    Event e;
    e.time=time;
    e.type=type;
    e.a=a;
    e.b=b;
    e.value=value;
    events.push_back(e);
}
void Simulation::processEvents(double upTo){
    now=upTo;
    int total=events.size();
    while(nextEvent<total && events[nextEvent].time<=upTo){
        Event e=events[nextEvent];
        nextEvent++;
        if(e.type==NEW_INCIDENT){
            Incident inc;
            inc.node=e.a;
            inc.severity=e.b;
            inc.arrivalTime=e.time;
            inc.dispatchTime=-1;
            inc.ambulanceId=-1;
            incidents.push_back(inc);
            int index=incidents.size()-1;
            //older incidents have a smaller arrival time so they rank higher
            waiting.push(index,inc.severity*SEVERITY_WEIGHT-inc.arrivalTime);
            cout<<"t="<<e.time<<": incident "<<index+1<<" (severity "<<inc.severity<<") at "<<g.getNode(inc.node).getName()<<endl;
        }
        else if(e.type==CLOSE_ROAD){
            g.setBlocked(e.a,e.b,true);
            cout<<"t="<<e.time<<": road "<<g.getNode(e.a).getName()<<" to "<<g.getNode(e.b).getName()<<" closed"<<endl;
        }
        else if(e.type==OPEN_ROAD){
            g.setBlocked(e.a,e.b,false);
            cout<<"t="<<e.time<<": road "<<g.getNode(e.a).getName()<<" to "<<g.getNode(e.b).getName()<<" reopened"<<endl;
        }
        else if(e.type==SET_TRAFFIC){
            g.setCongestion(e.a,e.b,e.value);
            cout<<"t="<<e.time<<": road "<<g.getNode(e.a).getName()<<" to "<<g.getNode(e.b).getName()<<" traffic x"<<e.value<<endl;
        }
    }
}
void Simulation::showDistances(int index){
    int target=incidents[index].node;
    int n=ambulances.size();
    cout<<"  distances to "<<g.getNode(target).getName()<<":"<<endl;
    for(int i=0;i<n;i++){
        if(ambulances[i].state!=IDLE){
            continue;
        }
        vector<double> dist;
        vector<int> prev;
        cout<<"    ambulance "<<ambulances[i].id<<": straight "<<g.straightKm(ambulances[i].node,target)<<" km";
        g.dijkstra(ambulances[i].node,dist,prev,false);
        cout<<", fixed weights "<<dist[target]<<" min";
        g.dijkstra(ambulances[i].node,dist,prev,true);
        cout<<", with traffic "<<dist[target]<<" min"<<endl;
    }
}
int Simulation::chooseAmbulance(int index, double& eta){
    int target=incidents[index].node;
    int n=ambulances.size();
    int best=-1;
    double bestScore=INF;
    for(int i=0;i<n;i++){
        if(ambulances[i].state!=IDLE){
            continue;
        }
        double score;
        if(strategy==NEAREST){
            score=g.straightKm(ambulances[i].node,target);
        }
        else{
            vector<double> dist;
            vector<int> prev;
            //traffic only counts for the dynamic strategy
            g.dijkstra(ambulances[i].node,dist,prev,strategy==DYNAMIC);
            score=dist[target];
        }
        if(score<bestScore){
            bestScore=score;
            best=i;
        }
    }
    if(best==-1){
        return -1;
    }
    //whatever the strategy, the eta is what the road really takes now
    vector<double> dist;
    vector<int> prev;
    g.dijkstra(ambulances[best].node,dist,prev,true);
    if(dist[target]>=INF){
        return -1;
    }
    eta=dist[target];
    return best;
}
void Simulation::assignAmbulances(){
    while(!waiting.isEmpty()){
        int index=waiting.top().id;
        double eta=0;
        int a=chooseAmbulance(index,eta);
        if(a==-1){
            break; //nobody free can reach it now, it keeps waiting
        }
        waiting.pop();
        showDistances(index); //before the state changes so the chosen one is still listed
        ambulances[a].state=TO_INCIDENT;
        ambulances[a].incident=index;
        incidents[index].dispatchTime=now;
        incidents[index].ambulanceId=ambulances[a].id;
        cout<<"  incident "<<index+1<<" -> ambulance "<<ambulances[a].id<<", eta "<<eta<<" min"<<endl;
    }
}
