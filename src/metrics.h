#pragma once
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
using namespace std;

// Everything we remember about one incident, filled in as the simulation runs.
struct IncidentRecord {
    int id = -1, node = -1, severity = 1;
    int ambulanceId = -1;
    int reroutes = 0;
    double arrival = 0;          // when it was reported
    double dispatched = -1;      // when an ambulance was assigned
    double reached = -1;         // when the ambulance arrived at the scene
    double delivered = -1;       // when the patient reached a hospital

    bool served() const { return reached >= 0; }
    double response() const { return reached - arrival; }      // report -> ambulance on scene
    double queueWait() const { return dispatched - arrival; }  // report -> assignment
};

class Metrics {
public:
    vector<IncidentRecord> records;
    double dispatchMicros = 0;   // wall-clock time spent choosing ambulances
    double routingMicros = 0;    // wall-clock time spent planning routes
    double endTime = 0;          // simulated minutes when the run finished

    int total() const { return (int)records.size(); }

    int served() const {
        int n = 0;
        for (const auto& r : records) if (r.served()) n++;
        return n;
    }

    int totalReroutes() const {
        int n = 0;
        for (const auto& r : records) n += r.reroutes;
        return n;
    }

    // Average response over served incidents with severity >= minSeverity. -1 if there are none.
    double avgResponse(int minSeverity = 1) const {
        double sum = 0; int n = 0;
        for (const auto& r : records)
            if (r.served() && r.severity >= minSeverity) { sum += r.response(); n++; }
        return n ? sum / n : -1;
    }

    double maxResponse() const {
        double m = -1;
        for (const auto& r : records) if (r.served()) m = max(m, r.response());
        return m;
    }

    void printTable(ostream& os) const {
        os << "\n  Inc  Node  Sev  Reported  Dispatched  On scene  Response  Amb  Reroutes  Delivered\n";
        os << "  ---  ----  ---  --------  ----------  --------  --------  ---  --------  ---------\n";
        os << fixed << setprecision(2);
        for (const auto& r : records) {
            os << "  " << setw(3) << r.id << "  " << setw(4) << r.node << "  " << setw(3) << r.severity
               << "  " << setw(8) << r.arrival;
            if (r.dispatched >= 0) os << "  " << setw(10) << r.dispatched; else os << "  " << setw(10) << "-";
            if (r.reached >= 0)    os << "  " << setw(8) << r.reached << "  " << setw(8) << r.response();
            else                   os << "  " << setw(8) << "-" << "  " << setw(8) << "-";
            if (r.ambulanceId >= 0) os << "  " << setw(3) << r.ambulanceId; else os << "  " << setw(3) << "-";
            os << "  " << setw(8) << r.reroutes;
            if (r.delivered >= 0)  os << "  " << setw(9) << r.delivered; else os << "  " << setw(9) << "-";
            os << "\n";
        }
    }
};
