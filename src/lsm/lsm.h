#ifndef LSM_H
#define LSM_H

#include <optional>
#include <vector>
#include "../sst/btree/btree.h"
#include "../memtable/memtable.h"
#include "../io/io.h"
#include <limits.h>

class Lsm {
    private:
        int size_ratio = 2;
        string dbname;
        int sentinel = INT_MIN;
        string level_prefix = "L";

        vector<shared_ptr<BTree>> trees;
        shared_ptr<Buffer> buffer;
        shared_ptr<Memtable> memtable;
        shared_ptr<IO> io;

    public:
        Lsm(string dbname, shared_ptr<Buffer> buffer, shared_ptr<Memtable> memtable,  shared_ptr<IO> io);
        void put(int key, int value);
        optional<int> get(int key);
        void del(int key);
        void load();
        void save();
};

#endif