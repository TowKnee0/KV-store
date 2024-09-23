#include "db1.h"
#include <iostream>
#include <chrono>

using namespace chrono;


int put_time(const unique_ptr<Db>& db, int count) {
    auto start = high_resolution_clock::now();
    for (int i = 0; i < count; i++) {
        db->put(rand(), rand());
    }
    return duration_cast<microseconds>(high_resolution_clock::now() - start).count();
} 

int get_time(const unique_ptr<Db>& db, int count) {
    auto start = high_resolution_clock::now();
    for (int i = 0; i < count; i++) {
        db->get(rand()); // latency so doesnt matter if its actually in there or not
    }
    return duration_cast<microseconds>(high_resolution_clock::now() - start).count();
}

int scan_time(const unique_ptr<Db>& db, int count) {
    int a, b, high, low;

    auto start = high_resolution_clock::now();
    for (int i = 0; i < count; i++) {
        // negligible overhead
        a = rand();
        b = rand();
        low = min(a, b);
        high = max(a, b);
        db->scan(low, high);
    }
    return duration_cast<microseconds>(high_resolution_clock::now() - start).count();
}

// TODO: should take in argc and argv to configure memtable size and number of loops
int main() {
    const int memtable_size = 1024;
    const int entry_size = 16;
    const int benchmark_interval = 52428800; // 50mb intervals
    const int entries_per_interval = benchmark_interval / entry_size;
    const int max_size = 1048576000; // 1gb

    unique_ptr<Db> db =  make_unique<Db1>(memtable_size);
    db->open("exp1");

    for (int i = 0; i < max_size / benchmark_interval; i++) {
        int us_put = put_time(db, entries_per_interval);
        int us_get = get_time(db, entries_per_interval);
        int us_scan = scan_time(db, entries_per_interval);

        cout << "Current db size: " << i * benchmark_interval << "bytes" << endl;
        cout << "put time(us): " << us_put << endl;
        cout << "get time(us): " << us_get << endl;
        cout << "scan time(us): " << us_scan << endl;
    }
}