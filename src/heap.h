#pragma once
#include <vector>
using namespace std;
struct HeapItem{
    int id; //node number or incident number
    double priority;
};
class Heap{
    vector<HeapItem> data;
    bool isMin; //true: smallest priority on top, false: largest on top
    bool better(double a, double b){
        //true if a should be above b
        if(isMin){
            return a<b;
        }
        return a>b;
    }
    void moveUp(int i){
        while(i>0){
            int parent=(i-1)/2;
            if(better(data[i].priority,data[parent].priority)){
                swap(data[i],data[parent]);
                i=parent;
            }
            else{
                break;
            }
        }
    }
    void moveDown(int i){
        int n=data.size();
        while(true){
            int left=2*i+1;
            int right=2*i+2;
            int best=i;
            if(left<n && better(data[left].priority,data[best].priority)){
                best=left;
            }
            if(right<n && better(data[right].priority,data[best].priority)){
                best=right;
            }
            if(best==i){
                break;
            }
            swap(data[i],data[best]);
            i=best;
        }
    }
    public:
    Heap(bool minHeap):isMin(minHeap){}
    bool isEmpty() const{
        return data.empty();
    }
    int size() const{
        return data.size();
    }
    HeapItem top() const{
        if(data.empty()){
            HeapItem none;
            none.id=-1;
            none.priority=0;
            return none;
        }
        return data[0];
    }
    void push(int id, double priority){
        HeapItem item;
        item.id=id;
        item.priority=priority;
        data.push_back(item);
        moveUp(data.size()-1);
    }
    HeapItem pop(){
        if(data.empty()){
            HeapItem none;
            none.id=-1;
            none.priority=0;
            return none;
        }
    HeapItem root=data[0];
        data[0]=data.back();
        data.pop_back();
        if(!data.empty()){
            moveDown(0);
        }
        return root;
    }
};
