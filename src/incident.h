#pragma once 
#include<string>
#include"heap.h"
using namespace std;

const double SEVERITY_WEIGHT=10.0; //points per seveirty rate 
const double AGING_RATE=1.0; //points lost per minute of arrival delay 

class Incident {
int id;
int node;            // intersection where the emergency is
int severity;        // 1 (minor) .. 5 (critical)
double arrivalTime;  // simulated minutes
public:
Incident() : id(-1), node(-1), severity(1), arrivalTime(0) {}
Incident(int id, int node, int severity, double arrivalTime): id(id), node(node), severity(severity), arrivalTime(arrivalTime) {}
int getId() const {
    return id;
     }
int getNode() const {
    return node;
    }
int getSeverity() const {
    return severity;
    }
double getArrivalTime() const {
     return arrivalTime;
    }
double key() const {                       // fixed priority key
    return severity * SEVERITY_WEIGHT - AGING_RATE * arrivalTime;
    }
    // how long it has waited (for display and the metrics later)
    double waited(double now) const { return now - arrivalTime; }
};

// comp(a, b) == true means "a has LOWER priority than b"
struct IncidentLower {
    bool operator()(const Incident& a, const Incident& b) const {
        if (a.key() != b.key()){
            return a.key() < b.key();
        }
        return a.getId() > b.getId();          // tie: earlier id goes first
    }
};

class IncidentQueue {
    Heap<Incident, IncidentLower> heap;
public:
    void add(const Incident& inc) { 
        heap.push(inc);
    }
    bool empty() const { 
        return heap.empty();
    }
    int size() const {
        return heap.size();
    }
    const Incident& peek() const {
        return heap.top();
    }
    Incident next() {                          // remove and return the most urgent
        Incident top = heap.top();
        heap.pop();
        return top;
    }
};