#include <iostream>
#include <iomanip>
#include "simulation.h"
using namespace std;
string strategyNames[4]={"","nearest","static route","dynamic"};
string scenarioNames[6]={"","light traffic","heavy congestion","road closes while driving","four incidents at once","traffic jam after the ambulance left"};
void printNumber(double x){
    cout<<setw(16);
    if(x<0){
        cout<<"-";
    }
    else{
        cout<<x;
    }
}
void compare(const Graph& map, int number){
    cout<<endl<<"scenario "<<number<<": "<<scenarioNames[number]<<endl;
    cout<<left<<setw(14)<<"strategy"<<right<<setw(16)<<"avg response"<<setw(16)<<"avg sev 4-5"<<setw(16)<<"longest"<<setw(16)<<"reroutes"<<setw(16)<<"algorithm (us)"<<endl;
    for(int s=1;s<=3;s++){
        Simulation sim(map,s);
        loadScenario(sim,number);
        cout.setstate(ios::failbit); //hide the decision log while comparing
        sim.run();
        cout.clear();
        cout<<fixed<<setprecision(2);
        cout<<left<<setw(14)<<strategyNames[s]<<right;
        printNumber(sim.avgResponse(1));
        printNumber(sim.avgResponse(4));
        printNumber(sim.maxResponse());
        cout<<setw(16)<<sim.getReroutes();
        printNumber(sim.algoMicros());
        cout<<endl;
    }
}
void runOne(const Graph& map, int number, int s){
    Simulation sim(map,s);
    loadScenario(sim,number);
    cout<<endl<<"scenario "<<number<<" ("<<scenarioNames[number]<<"), strategy: "<<strategyNames[s]<<endl;
    sim.run();
    cout<<endl;
    sim.printResults();
}
int main(){
    Graph map;
    if(!map.load("data/map.txt")){
        return 1;
    }
    int choice=-1;
    while(choice!=0){
        cout<<endl<<"1. compare the strategies on one scenario"<<endl;
        cout<<"2. compare the strategies on every scenario"<<endl;
        cout<<"3. run one strategy and show every decision"<<endl;
        cout<<"0. quit"<<endl;
        cout<<"choice: ";
        cin>>choice;
        if(choice==1){
            int number;
            cout<<"scenario (1-5): ";
            cin>>number;
            if(number>=1 && number<=5){
                compare(map,number);
            }
        }
        else if(choice==2){
            for(int number=1;number<=5;number++){
                compare(map,number);
            }
        }
        else if(choice==3){
            int number;
            int s;
            cout<<"scenario (1-5): ";
            cin>>number;
            cout<<"strategy (1 nearest, 2 static route, 3 dynamic): ";
            cin>>s;
            if(number>=1 && number<=5 && s>=1 && s<=3){
                runOne(map,number,s);
            }
        }
        else if(!cin){
            break; //input ended
        }
    }
    return 0;
}
