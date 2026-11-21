#include <iostream>
#include "heap.h"
using namespace std;
int main(){
    Heap minHeap(true);
    Heap maxHeap(false);
    double values[5]={5,2,9,1,7};
    for(int i=0;i<5;i++){
        minHeap.push(i,values[i]);
        maxHeap.push(i,values[i]);
    }
    cout<<"size: "<<minHeap.size()<<endl;
    cout<<"min heap (id:priority): ";
    while(!minHeap.isEmpty()){
        HeapItem item=minHeap.pop();
        cout<<item.id<<":"<<item.priority<<" ";
    }
    cout<<endl;
    cout<<"max heap (id:priority): ";
    while(!maxHeap.isEmpty()){
        HeapItem item=maxHeap.pop();
        cout<<item.id<<":"<<item.priority<<" ";
    }
    cout<<endl;
    HeapItem empty=minHeap.pop();
    cout<<"pop on empty heap gives id "<<empty.id<<endl;
    return 0;
}
