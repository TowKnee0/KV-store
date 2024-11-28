#include "lsm.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <queue>
#include <tuple>


Lsm::Lsm(string dbname, shared_ptr<Buffer> buffer, shared_ptr<Memtable> memtable, shared_ptr<IO> io, shared_ptr<BTreeFactory> factory, int filter_size_in_bits) {
    this->buffer = buffer;
    this->dbname = dbname;
    this->io = io;
    this->memtable = memtable;
    this->factory = factory;
    this->filter_size_in_bits = filter_size_in_bits;
}

void Lsm::put(int key, int value) {
    if (this->memtable->put(key, value) == EXIT_SUCCESS) {
        return;
    }

    vector<pair<int, int>> data = this->memtable->inOrder();
    this->memtable->clear();
    this->memtable->put(key, value);

    shared_ptr<BTree> temp = this->factory->create("temp", this->dbname, this->io, this->buffer, make_shared<BloomFilter>(this->filter_size_in_bits));
    temp->write(data);

    for (int i = 0; i < this->trees.size(); i++) {
        shared_ptr<BTree> tree = this->trees[i];
        int clevel = i + 1;
        // empty level
        if (tree == nullptr) {
            if (this->trees[clevel - 1] != nullptr) {
                throw runtime_error("Attempt to overwrite existing BTree at index: " + to_string(clevel - 1));
            }
            
            temp->rename(this->level_prefix + to_string(clevel));
            this->trees[clevel-1] = temp;            
            return;
        }

        if (tree->filename.find(this->level_prefix) != 0) {
            throw runtime_error("Invalid BTree name: '" + tree->filename +"' does not contain level_prefix");
        }

        int tlevel = stoi(tree->filename.substr(this->level_prefix.length()));
        shared_ptr<BTree> merged = this->factory->create("merged", this->dbname, this->io, this->buffer, make_shared<BloomFilter>(this->filter_size_in_bits));
        merged->mergeBTrees(tree->filename, temp->filename);


        // this->buffer->del(temp->filename);
        this->buffer->del(tree->filename);
        this->trees[tlevel-1] = nullptr;

        merged->rename("temp");
        temp = merged;
    }
    temp->rename(this->level_prefix + to_string(this->trees.size()+1));
    this->trees.push_back(temp);
}

void Lsm::del(int key) {
    this->put(key, this->sentinel);
}

optional<int> Lsm::get(int key) {
    optional<int> ret = this->memtable->get(key);
    if (ret.has_value() && ret.value() == INT_MIN) {
        return nullopt;
    } else if (ret.has_value()) {
        return ret.value();
    }

    for (shared_ptr<BTree> tree: this->trees) {
        if (tree == nullptr) {
            continue;
        }
        optional<int> ret = tree->get(key);
        if (ret.has_value() && ret.value() == INT_MIN) {
            return nullopt;
        } else if (ret.has_value()) {
            return ret.value();
        }
    }
    return nullopt;
}


void Lsm::load() {
    int mpages = io->numPagesInFile("memtable");
    if (mpages != -1) {
        for (int p  = 0; p < mpages; p++) {
            optional<vector<pair<int, int>>> data = this->io->readPageFromFile("memtable", p+1);
            if (!data.has_value()) {
                continue;
            }

            for (pair<int, int> pear : data.value()) {
                this->memtable->put(pear.first, pear.second);
            }
        }
        io->del("memtable");
    }
    
    vector<string> files = this->io->getDbFilenames();
    for (string file: files) {
        string f = file.substr(0, file.find('.'));
        if (f.find(this->level_prefix) != 0) {
            continue;
        }

        int tlevel = stoi(f.substr(this->level_prefix.length()));
        if (this->trees.size() < tlevel) {
            this->trees.resize(tlevel);
        }
        shared_ptr<BTree> tree = this->factory->create(f, this->dbname, this->io, this->buffer, make_shared<BloomFilter>(this->filter_size_in_bits));
        this->trees[tlevel - 1] = tree;
    }
}

vector<pair<int, int>> Lsm::scan(int start, int end) {
    vector<vector<pair<int, int>>> pages(this->trees.size() + 1);
    vector<int> curr_page(this->trees.size() + 1);
    vector<int> read_indexes(this->trees.size() + 1);
    // <next read key, recency, index>
    priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<tuple<int, int, int>>> pq; 
    vector<pair<int,int>> ret;
    int prev_key = INT_MIN;

    vector<pair<int, int>> table = this->memtable->scan(start, end);

    if (table.size() > 0) {
        pages[0] = table;
        pq.push({table[0].first, 0, 0});
    }

    for (int i = 0; i < this->trees.size(); i++) {
        if (trees[i] == nullptr) {
            continue;
        }

        vector<int> temp =  trees[i]->getKeyData(start);
        if (temp[0] != -1) {
            optional<vector<pair<int, int>>> page = this->buffer->getPage(trees[i]->getFilename(), temp[1]);
            if (!page.has_value()) {
                throw runtime_error("Invalid page returned from GetKeyData");
            }

            pages[i+1] = page.value();
            curr_page[i+1] = temp[1];
            read_indexes[i+1] = temp[0];
            pq.push({temp[2], i+1, i+1});
        }
    }

    while (!pq.empty()) {
        auto [key, recency, idx] = pq.top();
        pq.pop();
        
        if (key > end) {
            return ret;
        }

        if (key != prev_key) {
            if (pages[idx][read_indexes[idx]].second != INT_MIN) {
                ret.push_back(pages[idx][read_indexes[idx]]);
            }
            prev_key = key;
        }

        read_indexes[idx]++;
        // end of page, read next page if possible
        if (read_indexes[idx] >= pages[idx].size()) {
            if (idx == 0) { // memtable
                continue;
            }

            curr_page[idx]++;
            if (curr_page[idx] > trees[idx - 1]->getNumDataPages()) {
                continue;
            }

            optional<vector<pair<int, int>>> page = this->buffer->getPage(trees[idx-1]->getFilename(), curr_page[idx]);
            if (!page.has_value()) {
                continue;
            }
            pages[idx] = page.value();
            read_indexes[idx] = 0;
        }

        if (pages[idx][read_indexes[idx]].first <= end) {
            pq.push({pages[idx][read_indexes[idx]].first, recency, idx});
        }
    }
    return ret;
}

void Lsm::save() {
    vector<pair<int, int>> data = this->memtable->inOrder();
    this->io->writeToFile("memtable", data);
}