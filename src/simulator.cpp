#include "simulator.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <utility>
using namespace std;

namespace {
using Clock = chrono::steady_clock;

double microsSince(Clock::time_point t0) {
    return chrono::duration<double, micro>(Clock::now() - t0).count();
}
string num(double x) {                      // 2-decimal text for the log
    char b[32];
    snprintf(b, sizeof b, "%.2f", x);
    return b;
}
const double EPS = 1e-9;
}

// ------------------------------------------------------------------ setup

Simulator::Simulator(Graph graph, const Scenario& scenario, Strategy s, bool verbose)
    : g(move(graph)), sc(scenario), strategy_(s), verbose_(verbose) {
    for (const auto& p : sc.ambulances) fleet_.push_back(Ambulance(p.first, p.second));
    missions_.resize(fleet_.size());
    stable_sort(sc.events.begin(), sc.events.end(),
                [](const Event& x, const Event& y) { return x.time < y.time; });
}

string Simulator::routeStr(const vector<int>& path) const {
    string s;
    for (size_t i = 0; i < path.size(); i++) {
        if (i) s += " > ";
        s += nm(path[i]);
    }
    return s;
}

bool Simulator::roadBlocked(int u, int v) const {
    for (const Edge& e : g.neighbours(u))
        if (e.getTo() == v) return e.isBlocked();
    return false;
}

void Simulator::note(double t, const string& msg) {
    char buf[32];
    snprintf(buf, sizeof buf, "[t=%7.2f] ", t);
    string line = string(buf) + msg;
    log_.push_back(line);
    if (verbose_) cout << line << "\n";
}

// ------------------------------------------------------------ main loop

void Simulator::step(double dt) {
    processEvents();                                  // 1. things that happen now
    dispatchPending();                                // 2. assign free ambulances
    for (int i = 0; i < (int)fleet_.size(); i++)      // 3. move everyone
        advanceAmbulance(i, dt);
    now_ += dt;
}

bool Simulator::finished() const {
    if (nextEvent_ < sc.events.size()) return false;
    if (!queue_.empty()) return false;
    for (const Mission& m : missions_) if (m.active) return false;
    return true;
}

void Simulator::run(double dt, double maxTime) {
    while (!finished() && now_ < maxTime) step(dt);
    metrics_.endTime = now_;
    if (!finished()) note(now_, "Stopped at the time limit; some incidents were never served");
}

// --------------------------------------------------------------- events

void Simulator::processEvents() {
    while (nextEvent_ < sc.events.size() && sc.events[nextEvent_].time <= now_ + EPS) {
        const Event& e = sc.events[nextEvent_++];
        switch (e.type) {
        case EventType::INCIDENT: {
            Incident inc(nextIncidentId_++, e.a, e.severity, e.time);
            queue_.add(inc);
            IncidentRecord rec;
            rec.id = inc.getId(); rec.node = e.a; rec.severity = e.severity; rec.arrival = e.time;
            metrics_.records.push_back(rec);          // records[id - 1]
            note(e.time, "INCIDENT #" + to_string(inc.getId()) + " (severity " + to_string(e.severity) +
                         ") at " + nm(e.a) + ", queue size " + to_string(queue_.size()));
            break;
        }
        case EventType::BLOCK:
            g.setBlocked(e.a, e.b, true, e.both);
            note(e.time, "ROAD CLOSED: " + nm(e.a) + (e.both ? " <-> " : " -> ") + nm(e.b));
            break;
        case EventType::UNBLOCK:
            g.setBlocked(e.a, e.b, false, e.both);
            note(e.time, "ROAD REOPENED: " + nm(e.a) + (e.both ? " <-> " : " -> ") + nm(e.b));
            break;
        case EventType::CONGEST:
            g.setCongestion(e.a, e.b, e.factor);
            if (e.both) g.setCongestion(e.b, e.a, e.factor);
            note(e.time, "TRAFFIC: " + nm(e.a) + (e.both ? " <-> " : " -> ") + nm(e.b) +
                         " now x" + num(e.factor) + " slower");
            break;
        }
    }
}

// ------------------------------------------------------------- dispatch

void Simulator::dispatchPending() {
    while (!queue_.empty()) {
        bool anyIdle = false;
        for (const Ambulance& a : fleet_) if (a.isIdle()) { anyIdle = true; break; }
        if (!anyIdle) return;                         // everyone is busy: incidents wait

        const Incident inc = queue_.peek();           // copy: the heap entry is removed below

        auto t0 = Clock::now();
        DispatchResult r = dispatcher_.choose(g, fleet_, inc, strategy_);
        metrics_.dispatchMicros += microsSince(t0);

        if (!r.found()) {                             // no idle ambulance can reach it right now
            if (stalledIncident_ != inc.getId()) {
                stalledIncident_ = inc.getId();
                note(now_, "Incident #" + to_string(inc.getId()) + " cannot be reached right now, waiting");
            }
            return;
        }
        queue_.next();                                // now actually remove it

        int idx = 0;
        for (int i = 0; i < (int)fleet_.size(); i++)
            if (fleet_[i].getId() == r.ambulanceId) idx = i;

        if (verbose_) {                               // the "overall distances" table
            note(now_, "Distances to incident #" + to_string(inc.getId()) + " at " + nm(inc.getNode()) + ":");
            for (const Candidate& c : dispatcher_.candidates(g, fleet_, inc)) {
                string live = (c.liveMin >= INF) ? "unreachable" : num(c.liveMin) + " min";
                string stat = (c.staticMin >= INF) ? "unreachable" : num(c.staticMin) + " min";
                note(now_, "    Ambulance " + to_string(c.ambulanceId) + ": straight " + num(c.straightKm) +
                           " km | fixed-weight road " + stat + " | live road " + live);
            }
        }

        Mission& m = missions_[idx];
        m = Mission();
        m.active = true;
        m.inc = inc;
        m.target = inc.getNode();
        fleet_[idx].setStatus(AmbStatus::TO_INCIDENT);

        IncidentRecord& rec = metrics_.records[inc.getId() - 1];
        rec.dispatched = now_;
        rec.ambulanceId = r.ambulanceId;

        note(now_, "DISPATCH Ambulance " + to_string(r.ambulanceId) + " -> incident #" + to_string(inc.getId()) +
                   " (live ETA " + num(r.eta) + " min)");
        onArrive(idx, now_);                          // plans the first road (or arrives at once)
    }
}

// ------------------------------------------------------------- movement

// Called whenever an ambulance stands at an intersection: at dispatch and on every arrival.
void Simulator::onArrive(int idx, double clock) {
    Mission& m = missions_[idx];
    Ambulance& a = fleet_[idx];
    int here = a.getNode();

    if (here != m.target) {                           // not there yet: decide the next road
        decideNext(idx, clock);
        return;
    }

    IncidentRecord& rec = metrics_.records[m.inc.getId() - 1];
    if (a.getStatus() == AmbStatus::TO_INCIDENT) {
        rec.reached = clock;
        a.setStatus(AmbStatus::AT_SCENE);
        m.sceneLeft = SCENE_TIME;
        m.plan.clear();
        note(clock, "Ambulance " + to_string(a.getId()) + " ARRIVED at " + nm(here) +
                    " (response time " + num(rec.response()) + " min)");
    } else {
        finishMission(idx, clock);
    }
}

// The decision at an intersection: which road next?
//   DYNAMIC            : re-run Dijkstra on live weights every time.
//   NEAREST / STATIC   : keep the planned route, and only replan if the next road is closed.
bool Simulator::decideNext(int idx, double clock) {
    Mission& m = missions_[idx];
    Ambulance& a = fleet_[idx];
    int here = a.getNode();
    bool proactive = (strategy_ == Strategy::DYNAMIC);
    string who = "Ambulance " + to_string(a.getId()) + " at " + nm(here) + ": ";

    int plannedNext = (m.plan.size() >= 2 && m.plan[0] == here) ? m.plan[1] : -1;
    bool closedAhead = (plannedNext != -1 && roadBlocked(here, plannedNext));
    string how = "following the plan";

    if (proactive || plannedNext == -1 || closedAhead) {
        auto t0 = Clock::now();
        PathResult r = dijkstra(g, here, proactive);  // live weights only for DYNAMIC
        vector<int> p = buildPath(r, here, m.target);
        metrics_.routingMicros += microsSince(t0);

        if (p.size() < 2) {                           // no open route to the target
            if (!m.waiting) {
                m.waiting = true;
                note(clock, who + "no open route to " + nm(m.target) + ", waiting");
            }
            return false;
        }
        if (m.waiting) { m.waiting = false; note(clock, who + "a route is open again"); }

        if (plannedNext != -1 && p[1] != plannedNext) {   // the next road changed: a reroute
            metrics_.records[m.inc.getId() - 1].reroutes++;
            note(clock, who + "REROUTE, " + (closedAhead ? "road ahead is closed" : "a faster way exists") +
                        ": " + nm(plannedNext) + " -> " + nm(p[1]));
        }
        how = proactive ? "re-ran Dijkstra, live ETA left " + num(r.dist[m.target]) + " min"
                        : (plannedNext == -1 ? "planned at dispatch with fixed weights"
                                             : "replanned, road ahead closed");
        m.plan = p;
        plannedNext = p[1];
    }

    double minutes = -1;
    for (const Edge& e : g.neighbours(here))
        if (e.getTo() == plannedNext && !e.isBlocked()) { minutes = e.travelTime(); break; }
    if (minutes < 0) return false;                    // should not happen

    m.from = here;
    m.to = plannedNext;
    m.edgeTotal = m.edgeLeft = minutes;
    note(clock, who + "next -> " + nm(plannedNext) + " (" + num(minutes) + " min) [" + how + "]");
    return true;
}

bool Simulator::startHospitalLeg(int idx, double clock) {
    Mission& m = missions_[idx];
    Ambulance& a = fleet_[idx];
    int here = a.getNode();

    if (sc.hospitals.empty()) { finishMission(idx, clock); return true; }

    auto t0 = Clock::now();
    PathResult r = dijkstra(g, here, true);
    metrics_.routingMicros += microsSince(t0);

    int best = -1;
    double bestDist = INF;
    for (int h : sc.hospitals)
        if (r.dist[h] < bestDist) { bestDist = r.dist[h]; best = h; }

    if (best == -1) {
        if (!m.waiting) {
            m.waiting = true;
            note(clock, "Ambulance " + to_string(a.getId()) + " at " + nm(here) + ": no hospital reachable, waiting");
        }
        return false;
    }
    m.waiting = false;
    m.target = best;
    m.plan.clear();
    a.setStatus(AmbStatus::TO_HOSPITAL);
    note(clock, "Ambulance " + to_string(a.getId()) + " leaves the scene for " + nm(best) +
                " (live ETA " + num(bestDist) + " min)");
    onArrive(idx, clock);
    return true;
}

void Simulator::finishMission(int idx, double clock) {
    Mission& m = missions_[idx];
    Ambulance& a = fleet_[idx];
    metrics_.records[m.inc.getId() - 1].delivered = clock;
    a.setStatus(AmbStatus::IDLE);
    m.active = false;
    m.to = m.from = -1;
    note(clock, "Ambulance " + to_string(a.getId()) + " delivered the patient at " + nm(a.getNode()) +
                " and is free again");
}

// Move one ambulance forward by dt minutes, carrying leftover time across intersections.
void Simulator::advanceAmbulance(int idx, double dt) {
    Mission& m = missions_[idx];
    Ambulance& a = fleet_[idx];
    double budget = dt;
    double clock = now_;

    for (int guard = 0; budget > EPS && m.active && guard < 1000; guard++) {
        if (m.to != -1) {                                         // driving along a road
            double use = min(budget, m.edgeLeft);
            m.edgeLeft -= use; budget -= use; clock += use;
            if (m.edgeLeft > EPS) break;                          // still on this road
            a.setNode(m.to);                                      // reached the next intersection
            m.plan.erase(m.plan.begin());                         // plan[0] is the new current node
            m.from = m.to = -1;
            onArrive(idx, clock);
        } else if (a.getStatus() == AmbStatus::AT_SCENE) {        // with the patient
            double use = min(budget, m.sceneLeft);
            m.sceneLeft -= use; budget -= use; clock += use;
            if (m.sceneLeft > EPS) break;
            if (!startHospitalLeg(idx, clock)) break;
        } else {                                                  // stopped, no route yet
            if (!decideNext(idx, clock)) break;
        }
    }
}
