#pragma once
#include <string>
#include <vector>
#include <utility>
#include "graph.h"
using namespace std;

// Things that happen at a given simulated time.
enum class EventType { INCIDENT, BLOCK, UNBLOCK, CONGEST };

struct Event {
    double time = 0;                          // simulated minutes
    EventType type = EventType::INCIDENT;
    int a = 0, b = 0;                         // INCIDENT: a = node. Road events: a -> b
    int severity = 1;                         // INCIDENT only (1-5)
    double factor = 1.0;                      // CONGEST only (2.0 = twice as slow)
    bool both = false;                        // road events: also apply to b -> a
};

// One test case: where the ambulances start, where the hospitals are,
// and the timeline of incidents and road changes. Read from a text file.
class Scenario {
public:
    string name;
    vector<pair<int, int>> ambulances;        // (ambulance id, start node)
    vector<int> hospitals;                    // hospital nodes
    vector<Event> events;

    bool load(const string& file, string& err);
    bool validate(const Graph& g, string& err) const;   // checks node ids against the map
};
