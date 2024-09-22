#include "db1.h"

Db1::Db1(int memtable_size) {
    memtable_size = memtable_size;
}

void Db1::open(string name) {
    name = name;
    memtable = make_unique<Bst>(memtable_size);
}

void Db1::write_memtable() {
    vector<pair<int, int>> data = memtable->inOrder();

    num_sst++;
    unique_ptr<SST> s = make_unique<SST>(to_string(num_sst), name);
        
    s->write(data);
    sst.push_back(move(s));
    memtable->clear();
}

void Db1::put(int key, int value) {
    if (memtable->put(key, value) == TREE_FULL) {
        write_memtable();
        memtable->put(key, value);
    }
}

optional<int> Db1::get(int key) {
    if (auto v = memtable->get(key) != nullopt) {
        return v;
    }

    // iterate backwards through sst's since higher number => more recent
    for (auto it = sst.rbegin(); it != sst.rend(); ++it) {
         if(auto v = (*it)->get(key) != nullopt) {
            return v;
         }
    }
    return nullopt;
}

vector<pair<int, int>> Db1::scan(int key1, int key2){}

void Db1::close(){
    if (memtable->count() != 0) {
        write_memtable();
    }

    // TODO: do we need to write a metadata file do that we can
    // instantiate this when we open it again? i.e number of sst's
    // because we need to instantiate the sst's with dbname and filename
}