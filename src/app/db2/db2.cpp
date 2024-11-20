#include "db2.h"
#include "../../io/io_handler/io_handler.h"
#include "../../buffer/ext_hash/ext_hash.h"
#include "../../memtable/bst/bst.h"
#include "../../sst/btree/btree.h"
#include <queue>
#include <stdexcept>
#include <iostream>

using namespace std;

Db2::Db2(int memtable_size, int min_buffer_page_size, int max_buffer_page_size, float buffer_expand_factor, int buffer_frame_size, bool use_binary_search)
{
    this->memtable_size = memtable_size;
    this->min_buffer_page_size = min_buffer_page_size;
    this->max_buffer_page_size = max_buffer_page_size;
    this->buffer_expand_factor = buffer_expand_factor;
    this->buffer_frame_size = buffer_frame_size;
    this->use_binary_search = use_binary_search;
}

void Db2::open(string name)
{
    this->name = name;
    num_sst = 0;
    io_handler = make_shared<IOHandler>(name);
    memtable = make_unique<Bst>(memtable_size);
    buffer = make_shared<ExtHash>(min_buffer_page_size, max_buffer_page_size, buffer_expand_factor, buffer_frame_size, io_handler);

    // Populate the sst vector with all the sst files associated with this db
    // Assume that the sst files are sorted in ascending order of creation
    vector<string> filenames = io_handler->getDbFilenames();
    for (auto &f : filenames)
    {
        sst.push_back(make_unique<BTree>(f, name, io_handler, buffer));
    }
}

void Db2::write_memtable()
{
    cout << "writing memtable" << endl;
    vector<pair<int, int>> data = memtable->inOrder();
    cout << "1" << endl;

    unique_ptr<BTree> s = make_unique<BTree>(to_string(num_sst), name, io_handler, buffer);
    cout << "2" << endl;

    if (s->write(data) == EXIT_FAILURE)
    {
        throw runtime_error("memtable write failed");
    };

    cout << "3" << endl;

    sst.push_back(move(s));
    num_sst++;
    cout << "4" << endl;

    memtable->clear();
    cout << "finished writing memtable" << endl;
}

void Db2::put(int key, int value)
{

    if (memtable->put(key, value) == EXIT_FAILURE)
    {
        write_memtable();
        memtable->put(key, value);
    }
}

optional<int> Db2::get(int key)
{
    if (auto v = memtable->get(key); v != nullopt)
    {
        return v;
    }

    // iterate backwards through sst's since higher number => more recent
    for (auto it = sst.rbegin(); it != sst.rend(); ++it)
    {
        if (auto v = (*it)->get(key); v != nullopt)
        {
            return v;
        }
    }
    return nullopt;
}

vector<pair<int, int>> Db2::scan(int key1, int key2)
{

    // // Scan the memtable first
    // vector<pair<int, int>> memtable_entries = memtable->scan(key1, key2);

    // vector<vector<pair<int, int>>> results;

    // results.push_back(memtable_entries);
    // for (auto &s : sst)
    // {
    //     vector<pair<int, int>> entries = s->scan(key1, key2);
    //     if (entries.size() != 0)
    //     {
    //         results.push_back(entries);
    //     }
    // }
    // // create a vector which stores the current index of each sst
    // vector<int> indexes(results.size(), 0);

    // // Combine the results in sorted order.
    // vector<pair<int, int>> combined;

    // // Min heap stores the current key, with what pq it came from
    // priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> min_heap;

    // for (int i = 0; i < results.size(); i++)
    // {
    //     if (results[i].size() > 0)
    //     {
    //         min_heap.push({results[i].at(0).first, i});
    //     }
    // }

    // while (!min_heap.empty())
    // {
    //     auto [key, i] = min_heap.top();
    //     min_heap.pop();

    //     combined.push_back({key, results[i].at(indexes[i]).second});
    //     indexes[i]++;

    //     if (indexes[i] < results[i].size())
    //     {
    //         min_heap.push({results[i].at(indexes[i]).first, i});
    //     }
    // }

    // return combined;
    return {};
}

void Db2::change_buffer_size(int new_max_page_size)
{
    if (new_max_page_size < min_buffer_page_size)
    {
        throw runtime_error("new max page size is less than min page size");
    }
    buffer->updateMaximumSizeInPages(new_max_page_size);
}

void Db2::close()
{
    if (memtable->count() != 0)
    {
        write_memtable();
    }
}