#include "simulation.h"
#include <iostream>
using namespace std;
Simulation::Simulation(const Graph& map, int s):g(map),strategy(s),waiting(false){
    nextEvent=0;
    ticks=0;
    now=0;
    reroutes=0;
}
void Simulation::addAmbulance(int node){
    Ambulance a;
    a.id=ambulances.size()+1;
    a.node=node;
    a.state=IDLE;
    a.incident=-1;
    a.target=-1;
    a.nextNode=-1;
    a.timeLeft=0;
    a.roadTime=0;
    a.sceneLeft=0;
    a.stuck=false;
    ambulances.push_back(a);
}
void Simulation::addHospital(int node){
    hospitals.push_back(node);
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
    while(nextEvent<total && events[nextEvent].time<=upTo+0.0001){
        Event e=events[nextEvent];
        nextEvent++;
        if(e.type==NEW_INCIDENT){
            Incident inc;
            inc.node=e.a;
            inc.severity=e.b;
            inc.arrivalTime=e.time;
            inc.dispatchTime=-1;
            inc.reachTime=-1;
            inc.deliverTime=-1;
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
        ambulances[a].target=incidents[index].node;
        ambulances[a].route.clear();
        ambulances[a].nextNode=-1;
        ambulances[a].stuck=false;
        incidents[index].dispatchTime=now;
        incidents[index].ambulanceId=ambulances[a].id;
        cout<<"t="<<now<<": incident "<<index+1<<" -> ambulance "<<ambulances[a].id<<", eta "<<eta<<" min"<<endl;
        arrive(a);
    }
}
//called when ambulance a is standing at an intersection
void Simulation::arrive(int a){
    Ambulance& amb=ambulances[a];
    if(amb.node!=amb.target){
        decideNextRoad(a);
        return;
    }
    if(amb.state==TO_INCIDENT){
        incidents[amb.incident].reachTime=now;
        amb.state=AT_SCENE;
        amb.sceneLeft=SCENE_TIME;
        cout<<"t="<<now<<": ambulance "<<amb.id<<" reached "<<g.getNode(amb.node).getName()<<", response time "<<now-incidents[amb.incident].arrivalTime<<" min"<<endl;
    }
    else{
        incidents[amb.incident].deliverTime=now;
        cout<<"t="<<now<<": ambulance "<<amb.id<<" delivered the patient at "<<g.getNode(amb.node).getName()<<", free again"<<endl;
        amb.state=IDLE;
        amb.incident=-1;
    }
}
//the decision at an intersection: which road next?
void Simulation::decideNextRoad(int a){
    Ambulance& amb=ambulances[a];
    int here=amb.node;
    int planned=-1; //next node of the old plan
    if(amb.route.size()>=2 && amb.route[0]==here){
        planned=amb.route[1];
    }
    bool replan=false;
    if(strategy==DYNAMIC || planned==-1){
        replan=true; //dynamic always looks again, the others only plan once
    }
    else{
        for(const Edge& e: g.neighbours(here)){
            if(e.getTo()==planned && e.isBlocked()){
                replan=true; //road ahead is closed
            }
        }
    }
    double etaLeft=-1;
    if(replan){
        vector<double> dist;
        vector<int> prev;
        g.dijkstra(here,dist,prev,strategy==DYNAMIC);
        vector<int> path=g.getPath(prev,here,amb.target);
        if(path.size()<2){
            if(!amb.stuck){
                amb.stuck=true;
                cout<<"t="<<now<<": ambulance "<<amb.id<<" at "<<g.getNode(here).getName()<<" has no open route, waiting"<<endl;
            }
            return;
        }
        amb.stuck=false;
        if(planned!=-1 && path[1]!=planned){
            reroutes++;
            cout<<"t="<<now<<": ambulance "<<amb.id<<" reroutes at "<<g.getNode(here).getName()<<": "<<g.getNode(planned).getName()<<" -> "<<g.getNode(path[1]).getName()<<endl;
        }
        amb.route=path;
        planned=path[1];
        if(strategy==DYNAMIC){
            etaLeft=dist[amb.target];
        }
    }
    for(const Edge& e: g.neighbours(here)){
        if(e.getTo()==planned && !e.isBlocked()){
            amb.timeLeft=e.travelTime();
            amb.roadTime=e.travelTime();
        }
    }
    amb.nextNode=planned;
    cout<<"t="<<now<<": ambulance "<<amb.id<<" at "<<g.getNode(here).getName()<<": next -> "<<g.getNode(planned).getName()<<" ("<<amb.roadTime<<" min)";
    if(etaLeft>=0){
        cout<<", eta left "<<etaLeft<<" min";
    }
    cout<<endl;
}
void Simulation::goToHospital(int a){
    Ambulance& amb=ambulances[a];
    vector<double> dist;
    vector<int> prev;
    g.dijkstra(amb.node,dist,prev,true);
    int best=-1;
    int count=hospitals.size();
    for(int i=0;i<count;i++){
        if(dist[hospitals[i]]<INF && (best==-1 || dist[hospitals[i]]<dist[best])){
            best=hospitals[i];
        }
    }
    if(best==-1){
        if(!amb.stuck){
            amb.stuck=true;
            cout<<"t="<<now<<": ambulance "<<amb.id<<" cannot reach any hospital, waiting"<<endl;
        }
        return;
    }
    amb.stuck=false;
    amb.target=best;
    amb.route.clear();
    amb.state=TO_HOSPITAL;
    cout<<"t="<<now<<": ambulance "<<amb.id<<" leaves the scene for "<<g.getNode(best).getName()<<" (eta "<<dist[best]<<" min)"<<endl;
    arrive(a);
}
void Simulation::moveAmbulances(){
    int n=ambulances.size();
    for(int i=0;i<n;i++){
        Ambulance& amb=ambulances[i];
        if(amb.state==IDLE){
            continue;
        }
        if(amb.state==AT_SCENE){
            amb.sceneLeft-=TICK;
            if(amb.sceneLeft<=0.0001){
                goToHospital(i);
            }
        }
        else if(amb.nextNode==-1){
            decideNextRoad(i); //it was waiting for an open route
        }
        else{
            amb.timeLeft-=TICK;
            if(amb.timeLeft<=0.0001){
                amb.node=amb.nextNode;
                amb.nextNode=-1;
                amb.route.erase(amb.route.begin());
                arrive(i);
            }
        }
    }
}
void Simulation::step(){
    processEvents(now);
    assignAmbulances();
    ticks++;
    now=ticks*TICK;
    moveAmbulances();
}
bool Simulation::finished(){
    int total=events.size();
    if(nextEvent<total || !waiting.isEmpty()){
        return false;
    }
    int n=ambulances.size();
    for(int i=0;i<n;i++){
        if(ambulances[i].state!=IDLE){
            return false;
        }
    }
    return true;
}
void Simulation::run(){
    while(!finished() && now<600){
        step();
    }
    if(!finished()){
        cout<<"stopped at the time limit, some incidents were never served"<<endl;
    }
}
