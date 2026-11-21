#include "scenario.h"
#include <fstream>
#include <sstream>

bool Scenario::load(const string& file, string& err) {
    ifstream in(file);
    if (!in) { err = "Cannot open scenario file " + file; return false; }
    name = file;

    string line;
    int lineNo = 0;
    while (getline(in, line)) {
        lineNo++;
        size_t hash = line.find('#');                 // strip comments
        if (hash != string::npos) line = line.substr(0, hash);
        istringstream ss(line);
        string word;
        if (!(ss >> word)) continue;                  // blank line

        Event e;
        bool ok = true;
        if (word == "ambulance") {
            int id, node;
            ok = bool(ss >> id >> node);
            if (ok) ambulances.push_back({id, node});
        } 
        else if (word == "hospital") {
            int node;
            ok = bool(ss >> node);
            if (ok) hospitals.push_back(node);
        } else if (word == "incident") {
            e.type = EventType::INCIDENT;
            ok = bool(ss >> e.time >> e.a >> e.severity);
            if (ok && (e.severity < 1 || e.severity > 5)) ok = false;
            if (ok) events.push_back(e);
        } else if (word == "block" || word == "unblock") {
            e.type = (word == "block") ? EventType::BLOCK : EventType::UNBLOCK;
            int both = 0;
            ok = bool(ss >> e.time >> e.a >> e.b >> both);
            e.both = (both == 1);
            if (ok) events.push_back(e);
        } else if (word == "congest") {
            e.type = EventType::CONGEST;
            int both = 0;
            ok = bool(ss >> e.time >> e.a >> e.b >> e.factor);
            if (ok && e.factor <= 0) ok = false;
            ss >> both;                               // optional 5th value
            e.both = (both == 1);
            if (ok) events.push_back(e);
        } else {
            err = file + " line " + to_string(lineNo) + ": unknown keyword '" + word + "'";
            return false;
        }
        if (!ok) {
            err = file + " line " + to_string(lineNo) + ": bad values for '" + word + "'";
            return false;
        }
    }
    return true;
}

bool Scenario::validate(const Graph& g, string& err) const {
    auto good = [&](int n) { return n >= 0 && n < g.size(); };
    if (ambulances.empty()) { err = "Scenario has no ambulances"; return false; }
    for (auto& a : ambulances)
        if (!good(a.second)) { err = "Ambulance " + to_string(a.first) + " starts at an unknown node"; return false; }
    for (int h : hospitals)
        if (!good(h)) { err = "Hospital at an unknown node " + to_string(h); return false; }
    for (const Event& e : events) {
        if (!good(e.a) || (e.type != EventType::INCIDENT && !good(e.b))) {
            err = "An event refers to a node that is not in the map";
            return false;
        }
    }
    return true;
}
