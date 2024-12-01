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
        // Remove the .bin extension
        string new_filename = f.substr(0, f.find('.'));
        sst.push_back(make_unique<BTree>(new_filename, name, io_handler, buffer));
    }
}

void Db2::write_memtable()
{
    vector<pair<int, int>> data = memtable->inOrder();

    unique_ptr<BTree> s = make_unique<BTree>(to_string(num_sst), name, io_handler, buffer);

    if (s->write(data) == EXIT_FAILURE)
    {
        throw runtime_error("memtable write failed");
    };

    sst.push_back(move(s));
    num_sst++;

    memtable->clear();
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
            SST_to_PAGE[i] = buffer->getPage(sst[i]->getFilename(), page).value();
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

            SST_to_PAGE[popped_i] = buffer->getPage(sst[popped_i]->getFilename(), SST_to_PAGE_NUM[popped_i] + 1).value();
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

void Db2::del(int key) {}