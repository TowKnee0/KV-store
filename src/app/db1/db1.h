#include "../db.h"
#include "../../memtable/bst.h"
#include "../../sst/sst.h"

class Db1 : public Db {
    private:
        Bst memtable;
        vector<SST> sst;

    public: 
        void open(string name) const = 0;
        void put(int key, int value) const = 0;
        optional<int> get(int key) const = 0;
        vector<pair<int, int>> scan(int key1, int key2) const = 0;
        void close() const = 0;
};