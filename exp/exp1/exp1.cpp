#include "../../src/app/db1/db1.h"
#include <chrono>

// TODO: the data needs to be random

using namespace std;

void insert_data(Db& db, int count) {
    for (int i = 0, i < count; i++) {
        db.put(rand(), rand()); // TODO: do we need mod INT_MAX
    }
}

int latency_put(Db& db) {
    auto start = high_resolution_clock::now();  
    db.put(rand(), rand());
    return duration_cast<microseconds>(high_resolution_clock::now() - start);
} 

int latency_get(Db& db, int key) {
    auto start = high_resolution_clock::now();  
    db.get(rand()); // latency so doesnt matter if its actually in there or not
    return duration_cast<microseconds>(high_resolution_clock::now() - start);
}

int latency_scan(Db& db, int low, int high) {
    auto start = high_resolution_clock::now();  
    db.scan(low, high);
    return duration_cast<microseconds>(high_resolution_clock::now() - start);
}

// TODO: should take in argc and argv to configure memtable size and number of loops
int main() {
    int memtable_size = 1024;
    int insert_count = 1024;
    Db1 db = new Db1(memtable_size);
    db.open("exp1");

    for (int i = 0; i < 100; i++) {
        insert_data(db, i * (insert_count), insert_count - 1); // -1 since we are timing a single put
        cout << latency_put();

    }
}