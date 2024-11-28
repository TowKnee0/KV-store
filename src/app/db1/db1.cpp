#include "db1.h"
#include "../../io/io_handler/io_handler.h"
#include "../../memtable/bst/bst.h"
#include "../../sst/sorted_list/sorted_list.h"
#include <queue>
#include <iostream>
#include <stdexcept>

using namespace std;

Db1::Db1(int memtable_size)
{
    this->memtable_size = memtable_size;
}

void Db1::open(string name)
{
    this->name = name;
    num_sst = 0;
    io_handler = make_shared<IOHandler>(name);
    memtable = make_unique<Bst>(memtable_size);

    // Populate the sst vector with all the sst files associated with this db
    // Assume that the sst files are sorted in ascending order of creation
    vector<string> filenames = io_handler->getDbFilenames();
    for (auto &f : filenames)
    {
        sst.push_back(make_unique<SortedList>(f, name, io_handler));
    }
}

void Db1::write_memtable()
{
    vector<pair<int, int>> data = memtable->inOrder();

    num_sst++;
    unique_ptr<SortedList> s = make_unique<SortedList>(to_string(num_sst), name, io_handler);

    if (s->write(data) == EXIT_FAILURE)
    {
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

vector<pair<int, int>> Db1::scan(int key1, int key2)
{
    vector<vector<pair<int, int>>> SST_to_PAGE(sst.size() + 1);
    vector<int> SST_to_PAGE_index(sst.size() + 1);
    vector<int> SST_to_PAGE_NUM(sst.size() + 1);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> min_heap;

    for (int i = 0; i < sst.size(); i++)
    {
        vector<int> key_data = sst[i]->getKeyData(key1);
        int index = key_data[0];
        int page = key_data[1];
        int key = key_data[2];
        int value = key_data[3];

        if (page != -1 && key <= key2)
        {
            SST_to_PAGE[i] = io_handler->readPageFromFile(sst[i]->getFilename(), page).value();
            SST_to_PAGE_index[i] = index;
            min_heap.push({key, i});
            SST_to_PAGE_NUM[i] = page;
        }
        else 
        {
            SST_to_PAGE[i] = {};
            SST_to_PAGE_index[i] = -1;
            SST_to_PAGE_NUM[i] = -1;
        }
    }

    // Last entry in the vector is special since it doesnt store pages,
    // but instead stores the memtable entries
    vector<pair<int, int>> memtable_entries = memtable->scan(key1, key2);
    if (memtable_entries.size() != 0)
    {
        SST_to_PAGE[sst.size()] = memtable_entries;
        SST_to_PAGE_index[sst.size()] = 0;
        min_heap.push({memtable_entries[0].first, sst.size()});
    }
    else
    {
        SST_to_PAGE[sst.size()] = {};
        SST_to_PAGE_index[sst.size()] = -1;
        SST_to_PAGE_NUM[sst.size()] = -1;
    }

    vector<pair<int, int>> combined;

    while (!min_heap.empty())
    {
        pair<int, int> top_item = min_heap.top();
        int popped_key = top_item.first;
        int popped_i = top_item.second;
        min_heap.pop();

        combined.push_back({popped_key, SST_to_PAGE[popped_i][SST_to_PAGE_index[popped_i]].second});
        SST_to_PAGE_index[popped_i]++;

        if (SST_to_PAGE_index[popped_i] < SST_to_PAGE[popped_i].size())
        {
            // if next key within range
            if (SST_to_PAGE[popped_i][SST_to_PAGE_index[popped_i]].first > key2)
            {
                continue;
            }
            min_heap.push({SST_to_PAGE[popped_i][SST_to_PAGE_index[popped_i]].first, popped_i});
        }
        else // load next page
        {
            if (popped_i == SST_to_PAGE.size() - 1)
            {
                continue; // memtable, no more pages.
            }

            if (sst[popped_i]->getNumDataPages() < SST_to_PAGE_NUM[popped_i] + 1)
            {
                SST_to_PAGE[popped_i] = {};
                SST_to_PAGE_index[popped_i] = -1;
                SST_to_PAGE_NUM[popped_i] = -1;
                continue;
            }

            SST_to_PAGE[popped_i] = io_handler->readPageFromFile(sst[popped_i]->getFilename(), SST_to_PAGE_NUM[popped_i] + 1).value();
            SST_to_PAGE_index[popped_i] = 0;
            SST_to_PAGE_NUM[popped_i]++;
            if (SST_to_PAGE[popped_i].size() != 0 && SST_to_PAGE[popped_i][SST_to_PAGE_index[popped_i]].first <= key2)
            {
                min_heap.push({SST_to_PAGE[popped_i][SST_to_PAGE_index[popped_i]].first, popped_i});
            }
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
}
void Db1::del(int key) {}