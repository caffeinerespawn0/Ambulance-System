#include "raylib.h"
#include "simulation.h"
#include <cmath>
#include <string>
#include <vector>
#include<iostream>
using namespace std;
const int SCREEN_W=1100;
const int SCREEN_H=640;
const int MAP_W=720;
double scale=1;
double offsetX=0;
double offsetY=0;
vector<int> prevNode; //where each ambulance was after the last step
vector<int> prevNext;
vector<string> decisionLines; //the options at the last decision
vector<string> history; //short list of recent decisions
int lastReroutes=0;
bool paused=true;
bool pauseAtDecision=true;
int speed=20; //simulation steps per second
int strategy=DYNAMIC;
int scenario=5; //1 to 5, see loadScenario in simulation.cpp
double stepTimer=0;
string strategyNames[4]={"","nearest","static route","dynamic"};
string stateNames[4]={"idle","going to the incident","at the scene","going to the hospital"};
Color ambulanceColors[4]={BLUE,MAROON,DARKGREEN,PURPLE};
void drawText(const char* text,int x,int y,int size,Color color);
void setupScale(const Graph& g){
    double minX=g.getNode(0).getX();
    double maxX=minX;
    double minY=g.getNode(0).getY();
    double maxY=minY;
    for(int i=1;i<g.size();i++){
        double x=g.getNode(i).getX();
        double y=g.getNode(i).getY();
        if(x<minX){
            minX=x;
        }
        if(x>maxX){
            maxX=x;
        }
        if(y<minY){
            minY=y;
        }
        if(y>maxY){
            maxY=y;
        }
    }
    double w=maxX-minX;
    double h=maxY-minY;
    if(w<1){
        w=1;
    }
    if(h<1){
        h=1;
    }
    scale=(MAP_W-200)/w;
    if((SCREEN_H-200)/h<scale){
        scale=(SCREEN_H-200)/h;
    }
    offsetX=(MAP_W-w*scale)/2-minX*scale;
    offsetY=(SCREEN_H-h*scale)/2-minY*scale;
}
Vector2 position(const Graph& g, int node){
    Vector2 p;
    p.x=g.getNode(node).getX()*scale+offsetX;
    p.y=g.getNode(node).getY()*scale+offsetY;
    return p;
}
void drawDashed(Vector2 a, Vector2 b, Color c){
    for(int i=0;i<20;i=i+2){
        Vector2 p1={a.x+(b.x-a.x)*i/20,a.y+(b.y-a.y)*i/20};
        Vector2 p2={a.x+(b.x-a.x)*(i+1)/20,a.y+(b.y-a.y)*(i+1)/20};
        DrawLineEx(p1,p2,4,c);
    }
}
void drawArrow(Vector2 a, Vector2 b, Color c){
    float dx=b.x-a.x;
    float dy=b.y-a.y;
    float len=sqrt(dx*dx+dy*dy);
    dx=dx/len;
    dy=dy/len;
    Vector2 tip={b.x-dx*24,b.y-dy*24};
    Vector2 left={tip.x-dx*14+dy*8,tip.y-dy*14-dx*8};
    Vector2 right={tip.x-dx*14-dy*8,tip.y-dy*14+dx*8};
    DrawLineEx(tip,left,3,c);
    DrawLineEx(tip,right,3,c);
}
void drawRoads(const Graph& g){
    for(int u=0;u<g.size();u++){
        for(const Edge& e: g.neighbours(u)){
            int v=e.getTo();
            const Edge* back=NULL; //the road going the other way, if there is one
            for(const Edge& r: g.neighbours(v)){
                if(r.getTo()==u){
                    back=&r;
                }
            }
            if(back!=NULL && u>v){
                continue; //a two way road is drawn once
            }
            bool blocked=e.isBlocked();
            double slow=e.getCongestion();
            if(back!=NULL){
                if(back->isBlocked()){
                    blocked=true;
                }
                if(back->getCongestion()>slow){
                    slow=back->getCongestion();
                }
            }
            Vector2 a=position(g,u);
            Vector2 b=position(g,v);
            Color c=DARKGRAY;
            if(slow>1.0){
                c=ORANGE;
            }
            if(blocked){
                drawDashed(a,b,RED);
            }
            else{
                DrawLineEx(a,b,4,c);
            }
            if(back==NULL){
                drawArrow(a,b,c); //one way road
            }
            drawText(TextFormat("%.1f",e.travelTime()),(a.x+b.x)/2+6,(a.y+b.y)/2-18,16,DARKGRAY);
        }
    }
}
void drawRoutes(const Simulation& sim){
    const Graph& g=sim.getGraph();
    for(int i=0;i<sim.ambulanceCount();i++){
        const Ambulance& a=sim.getAmbulance(i);
        if(a.state==IDLE || a.state==AT_SCENE){
            continue;
        }
        Color c=Fade(ambulanceColors[i%4],0.45);
        int count=a.route.size();
        for(int k=0;k+1<count;k++){
            DrawLineEx(position(g,a.route[k]),position(g,a.route[k+1]),9,c);
        }
    }
}
void drawNodes(const Simulation& sim){
    const Graph& g=sim.getGraph();
    for(int n=0;n<g.size();n++){
        Color c=LIGHTGRAY;
        for(int h: sim.getHospitals()){
            if(h==n){
                c=GREEN;
            }
        }
        for(int i=0;i<sim.incidentCount();i++){
            const Incident& inc=sim.getIncident(i);
            if(inc.node==n && inc.deliverTime<0){
                if(inc.reachTime<0){
                    c=RED; //waiting for an ambulance
                }
                else{
                    c=YELLOW; //ambulance is here or has the patient
                }
            }
        }
        Vector2 p=position(g,n);
        DrawCircleV(p,20,c);
        DrawCircleLines(p.x,p.y,20,BLACK);
        drawText(TextFormat("%d",n),p.x-5,p.y-8,16,BLACK);
        const char* name=g.getNode(n).getName().c_str();
        int w=MeasureText(name,14);
        DrawRectangle(p.x-w/2-2,p.y+24,w+4,18,Fade(RAYWHITE,0.9));
        drawText(name,p.x-w/2,p.y+26,14,DARKGRAY);
    }
}
void drawAmbulances(const Simulation& sim){
    const Graph& g=sim.getGraph();
    for(int i=0;i<sim.ambulanceCount();i++){
        const Ambulance& a=sim.getAmbulance(i);
        Vector2 p=position(g,a.node);
        if(a.nextNode!=-1 && a.roadTime>0){
            Vector2 q=position(g,a.nextNode);
            double done=1-a.timeLeft/a.roadTime;
            p.x=p.x+(q.x-p.x)*done;
            p.y=p.y+(q.y-p.y)*done;
        }
        Color c=ambulanceColors[i%4];
        if(a.state==IDLE){
            c=GRAY;
        }
        DrawCircleV(p,14,c);
        DrawCircleLines(p.x,p.y,14,BLACK);
        drawText(TextFormat("A%d",a.id),p.x-10,p.y-7,14,WHITE);
    }
}
void drawPanel(Simulation& sim){
    int x=MAP_W+14;
    DrawLine(MAP_W,0,MAP_W,SCREEN_H,LIGHTGRAY);
    drawText(TextFormat("strategy: %s",strategyNames[strategy].c_str()),x,14,20,BLACK);
    drawText(TextFormat("time %.1f min    reroutes %d",sim.getNow(),sim.getReroutes()),x,42,18,DARKGRAY);
    if(sim.finished()){
        drawText("all incidents are done",x,68,18,DARKGREEN);
    }
    else if(paused){
        drawText("paused",x,68,18,MAROON);
    }
    else{
        drawText(TextFormat("running, %d steps per second",speed),x,68,18,DARKGREEN);
    }
    drawText("space: run / pause     n: next decision",x,96,14,GRAY);
    drawText("up / down: speed     d: pause at every decision",x,114,14,GRAY);
    drawText("1 nearest   2 static   3 dynamic   r restart",x,132,14,GRAY);
    drawText(TextFormat("tab: next scenario, now %d of 5",scenario),x,150,14,GRAY);
    int y=182;
    for(int i=0;i<sim.ambulanceCount();i++){
        const Ambulance& a=sim.getAmbulance(i);
        drawText(TextFormat("A%d: %s",a.id,stateNames[a.state].c_str()),x,y,16,ambulanceColors[i%4]);
        y=y+20;
    }
    y=y+12;
    if(decisionLines.size()==0){
        drawText("press space to start, or n for the",x,y,16,DARKGRAY);
        drawText("next decision",x,y+20,16,DARKGRAY);
    }
    for(int i=0;i<(int)decisionLines.size();i++){
        Color c=BLACK;
        if(decisionLines[i].find("chosen")!=string::npos){
            c=DARKGREEN;
        }
        if(decisionLines[i].find("REROUTE")!=string::npos){
            c=RED;
        }
        drawText(decisionLines[i].c_str(),x,y,16,c);
        y=y+22;
    }
    y=SCREEN_H-20-history.size()*18-24;
    drawText("recent decisions",x,y,16,BLACK);
    for(int i=0;i<(int)history.size();i++){
        y=y+18;
        drawText(history[i].c_str(),x,y+6,14,DARKGRAY);
    }
}
//builds the table of options for ambulance i at the intersection it just reached
void recordDecision(const Simulation& sim, int i, bool rerouted, double t){
    const Graph& g=sim.getGraph();
    const Ambulance& a=sim.getAmbulance(i);
    string head=TextFormat("t=%.1f  ambulance %d at %s",t,a.id,g.getNode(a.node).getName().c_str());
    if(rerouted){
        head=head+"  REROUTE";
    }
    decisionLines.clear();
    decisionLines.push_back(head);
    decisionLines.push_back("heading to "+g.getNode(a.target).getName());
    decisionLines.push_back("road + rest of the trip = total");
    for(const Edge& e: g.neighbours(a.node)){
        int v=e.getTo();
        string line="  "+g.getNode(v).getName();
        if(e.isBlocked()){
            line=line+": closed";
        }
        else{
            vector<double> dist;
            vector<int> prev;
            g.dijkstra(v,dist,prev,true);
            if(dist[a.target]>=INF){
                line=line+": no way to the target";
            }
            else{
                line=line+TextFormat(": %.1f + %.1f = %.1f",e.travelTime(),dist[a.target],e.travelTime()+dist[a.target]);
            }
        }
        if(v==a.nextNode){
            line=line+"   <- chosen";
        }
        decisionLines.push_back(line);
    }
    string shortLine=TextFormat("t=%.1f A%d at %s -> %s",t,a.id,g.getNode(a.node).getName().c_str(),g.getNode(a.nextNode).getName().c_str());
    if(rerouted){
        shortLine=shortLine+" (reroute)";
    }
    history.push_back(shortLine);
    if(history.size()>8){
        history.erase(history.begin());
    }
}
//one simulation step, returns true if an ambulance chose a road during it
bool stepOnce(Simulation& sim){
    double before=sim.getNow();
    sim.step();
    bool rerouted=sim.getReroutes()>lastReroutes;
    lastReroutes=sim.getReroutes();
    bool decided=false;
    for(int i=0;i<sim.ambulanceCount();i++){
        const Ambulance& a=sim.getAmbulance(i);
        if(a.nextNode!=-1 && (prevNext[i]==-1 || a.node!=prevNode[i])){
            double t=sim.getNow();
            if(a.node==prevNode[i]){
                t=before; //it was just sent out, the decision came at the start of the step
            }
            recordDecision(sim,i,rerouted,t);
            decided=true;
        }
        prevNode[i]=a.node;
        prevNext[i]=a.nextNode;
    }
    return decided;
}
void runToNextDecision(Simulation& sim){
    int count=0;
    while(count<5000 && !sim.finished()){
        count++;
        if(stepOnce(sim)){
            break;
        }
    }
}
void restart(Simulation& sim, const Graph& map){
    sim=Simulation(map,strategy);
    loadScenario(sim,scenario);
    prevNode.clear();
    prevNext.clear();
    for(int i=0;i<sim.ambulanceCount();i++){
        prevNode.push_back(sim.getAmbulance(i).node);
        prevNext.push_back(-1);
    }
    decisionLines.clear();
    history.clear();
    lastReroutes=0;
    stepTimer=0;
    paused=true;
}
Font mainFont; 
void drawText(const char* text,int x,int y,int size,Color color)
{
    DrawTextEx(mainFont,text,{(float)x,(float)y},size+3,1,color);
}
int main(){
    Graph map;
    if(!map.load("data/map.txt")){
        return 1;
    }
    setupScale(map);
    Simulation sim(map,strategy);
    restart(sim,map);
    InitWindow(SCREEN_W,SCREEN_H,"smart ambulance dispatch");
    mainFont=LoadFontEx("assets/segoeui.ttf",48,0,250);
    if(mainFont.texture.id==0)
    {
    cout<<"font failed to load"<<endl;
    }
    else
    {
    cout<<"font loaded successfully"<<endl;
    }
    SetTargetFPS(60);
    while(!WindowShouldClose()){
        if(IsKeyPressed(KEY_SPACE)){
            paused=!paused;
        }
        if(IsKeyPressed(KEY_UP) && speed<320){
            speed=speed*2;
        }
        if(IsKeyPressed(KEY_DOWN) && speed>5){
            speed=speed/2;
        }
        if(IsKeyPressed(KEY_D)){
            pauseAtDecision=!pauseAtDecision;
        }
        if(IsKeyPressed(KEY_ONE)){
            strategy=NEAREST;
            restart(sim,map);
        }
        if(IsKeyPressed(KEY_TWO)){
            strategy=STATIC_ROUTE;
            restart(sim,map);
        }
        if(IsKeyPressed(KEY_THREE)){
            strategy=DYNAMIC;
            restart(sim,map);
        }
        if(IsKeyPressed(KEY_R)){
            restart(sim,map);
        }
        if(IsKeyPressed(KEY_TAB)){
            scenario=scenario%5+1;
            restart(sim,map);
        }
        if(IsKeyPressed(KEY_N) || IsKeyPressed(KEY_RIGHT)){
            runToNextDecision(sim);
        }
        if(!paused && !sim.finished()){
            stepTimer=stepTimer+GetFrameTime()*speed;
            while(stepTimer>=1 && !paused){
                stepTimer=stepTimer-1;
                if(stepOnce(sim) && pauseAtDecision){
                    paused=true;
                }
            }
        }
        else{
            stepTimer=0;
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawRoads(sim.getGraph());
        drawRoutes(sim);
        drawNodes(sim);
        drawAmbulances(sim);
        drawPanel(sim);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
