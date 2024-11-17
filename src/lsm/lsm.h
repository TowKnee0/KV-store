#ifndef LSM_H
#define LSM_H

#include <optional>
#include <vector>
#include "../sst/btree/btree.h"
#include "../filter/bloom_filter/bloom_filter.h"
#include "../memtable/memtable.h"
#include "../sst/btree/factory.h"
#include "../io/io.h"
#include <limits.h>
#include <gtest/gtest.h>

class Lsm {
    private:
        int size_ratio = 2;
        int filter_size_in_bits;
        string dbname;
        int sentinel = INT_MIN;
        string level_prefix = "L";

        shared_ptr<BTreeFactory> factory;
        vector<shared_ptr<BTree>> trees;
        shared_ptr<Buffer> buffer;
        shared_ptr<Memtable> memtable;
        shared_ptr<IO> io;

        FRIEND_TEST(LsmTest, PutHandlesBTreeMerge);
        FRIEND_TEST(LsmTest, PutfreeSpot);

    public:
        Lsm(string dbname, shared_ptr<Buffer> buffer, shared_ptr<Memtable> memtable,  shared_ptr<IO> io, shared_ptr<BTreeFactory> factory, int filter_size_in_bits);
        void put(int key, int value);
        optional<int> get(int key);
        void del(int key);
        void load();
        void save();
    };

#endif

