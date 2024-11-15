#ifndef LSM_H
#define LSM_H

#include <optional>
#include <vector>
#include "../sst/btree/btree.h"
#include "../memtable.h"



class Lsm {
    private:
        int size_ratio = 2;
        vector<unique_ptr<BTree>> trees;
        shared_ptr<Memtable> buffer;

    public:
        Lsm(shared_ptr<Memtable> buffer);
        void put(int key, int value);
        optional<int> get();
        void del(int key);
};

#endif