#include <iostream>
#include "incident.h"
using namespace std; 
int main() {
    IncidentQueue q; //heap of incidents (incident queue is a heap struct as declared in incident.h) 
    //id node severity arrival is the format: 
    
//to remember: incident is an object which is being added into the heap (q) !!! 

    q.add(Incident(1, 7, 2, 0));    // formula of key: (incident.h)
    q.add(Incident(2, 5, 5, 3));    // key=severity*10-1.0*arrivalTime 
    q.add(Incident(3, 4, 3, 1));    // arrival in minutes meaning: 0 min, 3 min, 1 min, 0 min
    q.add(Incident(4, 6, 1, 0));    // example: key 10
    while (!q.empty()) {
        Incident i = q.next(); //fifo structure
        cout << "incident " << i.getId() << " : Severity " << i.getSeverity() <<endl;
    }

    cout << "     Aging    "<<endl;
    q.add(Incident(10, 1, 1, 0));   // low severity, waited long: key 10
    q.add(Incident(11, 2, 2, 15));  // higher severity, arrived late: key 5
    while (!q.empty()) {
        Incident i = q.next();
        cout << "incident " << i.getId() << "\n";
    }
    // expected: 10 then 11 (the old minor case beats the newer, more severe one)
}