#include "../db.h"
#include "../../memtable/bst.h"
#include "../../sst/sorted_list/sorted_list.h"
#include <memory>

using namespace std;

class Db1 : public Db {
    private:
        string name;
        int memtable_size;
        unique_ptr<Bst> memtable;
        
        int num_sst;
        vector<unique_ptr<SortedList>> sst;
        shared_ptr<IOHandler> io_handler;
        
        void write_memtable();

    // TODO: check which of these can be shared and put it as non-virtual in ../db.h
    public:
        Db1(int memtable_size);
        void open(string name);
        void put(int key, int value);
        optional<int> get(int key);
        vector<pair<int, int>> scan(int key1, int key2);
        void close();
};