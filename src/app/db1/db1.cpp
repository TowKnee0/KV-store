#include "db1.h"

Db1::Db1(int memtable_size) {
    memtable_size = memtable_size;
}

void Db1::open(string name) {
    name = name;
    memtable = make_unique<Bst>(memtable_size);
}

void Db1::put(int key, int value) {
    if (memtable->put(key, value) == TREE_FULL) {
        vector<pair<int, int>> data = memtable->inOrder();
        unique_ptr<SST> s = make_unique<SST>(to_string(num_sst), name);
        
        s->write(data);
        sst.push_back(move(s));
        num_sst++;

        memtable->clear();
        memtable->put(key, value);
    }
}