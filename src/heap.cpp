#include <iostream>
#include "heap.h"
int main() {
    Heap<int, greater<int>> h;                 // min-heap
    for (int x : {5, 2, 9, 1, 7}) {
        h.push(x);
    }
    while (!h.empty()){
    cout << h.top() << " "; h.pop();
    }   // 1 2 5 7 9
}