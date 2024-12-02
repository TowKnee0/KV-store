#include "db2.h"
#include <iostream>
#include <chrono>

using namespace chrono;

duration<double> put(const unique_ptr<Db> &db, int count)
{
    auto start = high_resolution_clock::now();
    for (int i = 0; i < count; i++)
    {
        db->put(rand(), rand());
    }
    return high_resolution_clock::now() - start;
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
        // fixed range scan
        a = rand() % (RAND_MAX - 10000);
        b = a + 10000;

        low = min(a, b);
        high = max(a, b);
        db->scan(low, high);
        count++;
    }
    return count;
}


int main()
{
    auto test_start_time = high_resolution_clock::now();

    const int megabyte = 1048576;
    const int gigabyte = 1073741824;
    // 4 bytes per int, 8 bytes per entry
    const int entry_size = 8;
    // Memtable takes in number of entries
    const int memtable_size = megabyte / entry_size;

    const int buffer_pool_pages = (10*megabyte) / 4096; // 4kb pages

    const int benchmark_interval = 100*megabyte;        // 100mb intervals
    const int entries_per_interval = benchmark_interval / entry_size;
    const int max_size = gigabyte;
    const seconds duration(5);
    const int duration_int = static_cast<int>(duration.count());

    unique_ptr<Db> db = make_unique<Db2>(memtable_size, 1, buffer_pool_pages, 0.8, 3, false);
    db->open("exp2");
    cout << "Current size(bytes),put/s,get/s,scan/s" << endl;

    for (int i = 0; i < max_size / benchmark_interval; i++)
    {
        auto put_time = put(db, benchmark_interval / entry_size);
        float put_per_sec = (benchmark_interval / entry_size) / static_cast<float>(duration_cast<seconds>(put_time).count());

        float num_get = count_get(db, duration);
        float num_scan = count_scan(db, duration);

        int c_size = (i+1) * benchmark_interval;
        cout << c_size << "," << put_per_sec << "," << num_get / duration_int << "," << num_scan / duration_int << endl;
    }
    
    db->close();
    auto test_end_time = high_resolution_clock::now();
    auto test_duration = duration_cast<seconds>(test_end_time - test_start_time);
    cout << "Test duration: " << test_duration.count() << " seconds" << endl;
    cout << "Test duration in minutes " << test_duration.count() / 60 << " minutes" << endl;
}