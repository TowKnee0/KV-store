#include "db1.h"
#include "../../lib/io/io.h"
#include <queue>
#include <iostream>

using namespace std;

Db1::Db1(int memtable_size)
{
    memtable_size = memtable_size;
}

void Db1::open(string name)
{
    name = name;
    shared_ptr<IOHandler> io_handler = make_shared<IOHandler>(name);
    memtable = make_unique<Bst>(memtable_size);

    // Populate the sst vector with all the sst files associated with this db
    // Assume that the sst files are sorted in ascending order of creation
    vector<string> filenames = io_handler->getDbFilenames();
    for (auto &f : filenames)
    {
        sst.push_back(make_unique<Sst>(f, name, io_handler));
    }
}

void Db1::write_memtable()
{
    vector<pair<int, int>> data = memtable->inOrder();

    num_sst++;
    shared_ptr<IOHandler> io_handler = make_shared<IOHandler>(name);
    unique_ptr<Sst> s = make_unique<Sst>(to_string(num_sst), name, io_handler);

    if (s->write(data) == EXIT_FAILURE){
        throw runtime_error("memtable write failed");
    };
    sst.push_back(move(s));
    memtable->clear();
}

void Db1::put(int key, int value)
{
    if (memtable->put(key, value) == EXIT_FAILURE)
    {
        write_memtable();
        memtable->put(key, value);
    }
}

optional<int> Db1::get(int key)
{
    if (auto v = memtable->get(key) != nullopt)
    {
        return v;
    }

    // iterate backwards through sst's since higher number => more recent
    for (auto it = sst.rbegin(); it != sst.rend(); ++it)
    {
        if (auto v = (*it)->get(key) != nullopt)
        {
            return v;
        }
    }
    return nullopt;
}

vector<pair<int, int>> Db1::scan(int key1, int key2)
{
    // Step 1: Create a vector to store the result of each sst scan.
    // Step 2: scan each sst, updating the vector.
    // Combine the results in sorted order and return the result.

    vector<vector<entry> *> results;
    for (auto &s : sst)
    {
        vector<entry> *entries = s->scan(key1, key2);
        if (entries != nullptr)
        {
            results.push_back(entries);
        }
    }

    // create a vector which stores the current index of each sst
    vector<int> indexes(results.size(), 0);

    // Combine the results in sorted order.
    vector<pair<int, int>> combined;

    // Min heap stores the current key, with what pq it came from
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> min_heap;

    for (int i = 0; i < results.size(); i++)
    {
        if (results[i]->size() > 0)
        {
            min_heap.push({results[i]->at(0).key, i});
        }
    }

    while (!min_heap.empty())
    {
        auto [key, i] = min_heap.top();
        min_heap.pop();

        combined.push_back({key, results[i]->at(indexes[i]).value});
        indexes[i]++;

        if (indexes[i] < results[i]->size())
        {
            min_heap.push({results[i]->at(indexes[i]).key, i});
        }
    }

    return combined;
}

void Db1::close()
{
    if (memtable->count() != 0)
    {
        write_memtable();
    }

    // TODO: do we need to write a metadata file do that we can
    // instantiate this when we open it again? i.e number of sst's
    // because we need to instantiate the sst's with dbname and filename
}