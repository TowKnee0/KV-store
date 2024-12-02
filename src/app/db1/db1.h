#include "../db.h"
#include "../../memtable/memtable.h"
#include "../../sst/sst.h"
#include "../../io/io.h"
#include <memory>

using namespace std;

class Db1 : public Db {
    private:
        string name;
        int memtable_size;
        unique_ptr<Memtable> memtable;
        
        int num_sst;
        vector<unique_ptr<Sst>> sst;
        shared_ptr<IO> io_handler;
        
        void write_memtable();


    public:
        Db1(int memtable_size);
        void open(string name);
        void put(int key, int value);
        optional<int> get(int key);
        vector<pair<int, int>> scan(int key1, int key2);
        void del (int key);
        void close();
};