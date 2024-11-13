#include "../db.h"
#include "../../memtable/memtable.h"
#include "../../sst/sst.h"
#include "../../io/io.h"
#include "../../buffer/buffer.h"
#include <memory>

using namespace std;

class Db2 : public Db {
    private:
        string name;
        bool use_binary_search;

        int memtable_size;
        unique_ptr<Memtable> memtable;
        shared_ptr<Buffer> buffer;
        int min_buffer_page_size;
        int max_buffer_page_size;
        float buffer_expand_factor;
        int buffer_frame_size;

        shared_ptr<IO> io_handler;
        
        int num_sst;
        vector<unique_ptr<Sst>> sst;
        
        void write_memtable();

    
    public:
        Db2(int memtable_size, int min_buffer_page_size, int max_buffer_page_size, float buffer_expand_factor, int buffer_frame_size, bool use_binary_search);
        void open(string name);
        void put(int key, int value);
        optional<int> get(int key);
        vector<pair<int, int>> scan(int key1, int key2);
        void change_buffer_size(int new_max_page_size);
        void close();
};