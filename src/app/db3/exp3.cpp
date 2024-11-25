#include "db3.h"
#include <iostream>
#include <chrono>

using namespace chrono;

duration<double> put(const unique_ptr<Db> &db, int count)
{
    auto start = high_resolution_clock::now();
    for (int i = 0; i < count; i++)
    {
        cout << "put: " << i << " " << count << endl;
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
        a = rand();
        b = rand();
        low = min(a, b);
        high = max(a, b);
        db->scan(low, high);
        count++;
    }
    return count;
}

int main()
{
    const int megabyte = 1048576;
    const int gigabyte = 1073741824;
    // 4 bytes per int, 8 bytes per entry
    const int entry_size = 8;
    // Memtable takes in number of entries
    const int memtable_size = megabyte / entry_size;

    const int buffer_pool_pages = (10*megabyte) / 4096; // 4kb pages
    const int filter_bits_per_entry = 8;

    const int benchmark_interval = 100*megabyte;        // 100mb intervals
    const int entries_per_interval = benchmark_interval / entry_size;
    const int max_size = gigabyte;
    const seconds duration(5);
    const int duration_int = static_cast<int>(duration.count());

    unique_ptr<Db> db = make_unique<Db3>(memtable_size, 1, buffer_pool_pages, 0.8, 3, filter_bits_per_entry, false);

    cout << "Open Db" << endl;

    db->open("exp3");

    for (int i = 0; i < max_size / benchmark_interval; i++)
    {
        // insert fixed amount of data
        auto put_time = put(db, benchmark_interval / entry_size);
        // then get/scan for fixed duration to not make experiment too long
        int num_get = count_get(db, duration);
        int num_scan = count_scan(db, duration);

        cout << "Current db size, " << (i+1) * benchmark_interval << " bytes" << endl;
        cout << "num_put/s, " << (benchmark_interval / entry_size) / static_cast<int>(duration_cast<seconds>(put_time).count()) << endl;
        cout << "num_get/s, " << num_get / duration_int << endl;
        cout << "num_scan/s, " << num_scan / duration_int << endl;
    }
    cout << "closing db" << endl;
    db->close();
}