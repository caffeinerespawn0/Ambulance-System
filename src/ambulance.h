#pragma once
enum class AmbStatus { 
    IDLE, TO_INCIDENT, AT_SCENE, TO_HOSPITAL
};

class Ambulance {
    int id;
    int node;            // current intersection
    AmbStatus status;

public:
    Ambulance(int id, int node) : id(id), node(node), status(AmbStatus::IDLE) {}; 
    int getId()const{ 
        return id;
    }
    int getNode()const{
        return node;
    }
    AmbStatus getStatus() const{
        return status;
    }
    bool isIdle() const{
        return status == AmbStatus::IDLE;
    }
    void setNode(int n) { 
        node = n;
    }
    void setStatus(AmbStatus s){
        status = s;
    }
};