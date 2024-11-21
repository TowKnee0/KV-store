#include "db3.h"
#include <iostream>
#include <chrono>

using namespace chrono;

int count_put(const unique_ptr<Db> &db, seconds s)
{
    auto stop = high_resolution_clock::now() + s;
    int count = 0;
    while (high_resolution_clock::now() < stop)
    {
        db->put(rand(), rand());
        count++;
    }
    return count;
}

int count_get(const unique_ptr<Db> &db, seconds s)
{
    auto stop = high_resolution_clock::now() + s;
    int count = 0;
    while (high_resolution_clock::now() < stop)
    {
        db->get(rand()); // latency so doesnt matter if its actually in there or not
        count++;
    }
    return count;
}

int count_scan(const unique_ptr<Db> &db, seconds s)
{
    int a, b, high, low;

    auto stop = high_resolution_clock::now() + s;
    int count = 0;
    while (high_resolution_clock::now() < stop)
    {
        a = rand();
        b = rand();
        low = min(a, b);
        high = max(a, b);
        db->scan(low, high);
        count++;
    }
    return count;
}

// TODO: should take in argc and argv to configure memtable size and number of loops
int main()
{
    const int entry_size = 16;
    const int memtable_size = 1048576 / entry_size; // 5mb
    const int benchmark_interval = 10485760;        // 1mb intervals
    const int entries_per_interval = benchmark_interval / entry_size;
    const int max_size = 10485760; // 50mb
    const seconds duration(2);
    const int duration_int = static_cast<int>(duration.count());

    unique_ptr<Db> db = make_unique<Db3>(memtable_size, 1, 100, 0.8, 3, 8, false);
    db->open("exp3");

    for (int i = 0; i < max_size / benchmark_interval; i++)
    {
        int num_put = count_put(db, duration);
        int num_get = count_get(db, duration);
        int num_scan = count_scan(db, duration);

        cout << "Current db size: " << num_put * 16 << "bytes" << endl;
        cout << "num_put/s: " << num_put / duration_int << endl;
        cout << "num_get/s: " << num_get / duration_int << endl;
        cout << "num_scan/s: " << num_scan / duration_int << endl;
    }
    cout << "closing db" << endl;
    db->close();
}