#pragma once
#include <vector>
#include <functional>
#include <utility>
using namespace std;

// Binary heap stored in a vector.
// comp(a, b) == true means "a has LOWER priority than b" (same rule as std::priority_queue).
//   Heap<int>                   -> max-heap
//   Heap<int, greater<int>>     -> min-heap
template <typename T, typename Compare = less<T>>
class Heap {
    vector<T> data;
    Compare comp;

    void heapifyUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (comp(data[p], data[i])) { swap(data[p], data[i]); i = p; }
            else break;
        }
    }
    void heapifyDown(int i) {
        int n = data.size();
        while (true) {
            int l = 2 * i + 1, r = 2 * i + 2, best = i;
            if (l < n && comp(data[best], data[l])) best = l;
            if (r < n && comp(data[best], data[r])) best = r;
            if (best == i) break;
            swap(data[i], data[best]);
            i = best;
        }
    }
public:
    bool empty() const { return data.empty(); }
    int size() const { return data.size(); }
    const T& top() const { return data[0]; }          // peek, O(1)

    void push(const T& x) {                           // insert, O(log n)
        data.push_back(x);
        heapifyUp(data.size() - 1);
    }
    void pop() {                                      // extract top, O(log n)
        data[0] = data.back();
        data.pop_back();
        if (!data.empty()) heapifyDown(0);
    }
};