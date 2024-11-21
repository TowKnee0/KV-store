#include "db3.h"
#include "../../io/io_handler/io_handler.h"
#include "../../buffer/ext_hash/ext_hash.h"
#include "../../memtable/bst/bst.h"
#include "../../sst/btree/factory.h"



Db3::Db3(int memtable_size, int min_buffer_page_size, int max_buffer_page_size, float buffer_expand_factor, int buffer_frame_size, int bloom_filter_bits, bool use_binary_search)
{
    this->memtable_size = memtable_size;
    this->min_buffer_page_size = min_buffer_page_size;
    this->max_buffer_page_size = max_buffer_page_size;
    this->buffer_expand_factor = buffer_expand_factor;
    this->buffer_frame_size = buffer_frame_size;
    this->bloom_filter_bits = bloom_filter_bits;
    this->use_binary_search = use_binary_search;
}

void Db3::open(string name)
{
    this->name = name;
    io_handler = make_shared<IOHandler>(name);
    memtable = make_shared<Bst>(memtable_size);
    buffer = make_shared<ExtHash>(min_buffer_page_size, max_buffer_page_size, buffer_expand_factor, buffer_frame_size, io_handler);
    factory = make_shared<DefaultBTreeFactory>();
    lsm = make_shared<Lsm>(name, buffer, memtable, io_handler, factory, bloom_filter_bits);

    lsm->load();
}

void Db3::put(int key, int val) {
    lsm->put(key, val);
}

optional<int> Db3::get(int key) {
    return lsm->get(key);
}

vector<pair<int, int>> Db3::scan(int start, int end) {
    return lsm->scan(start, end);
}

void Db3::change_buffer_size(int new_max_page_size)
{
    if (new_max_page_size < min_buffer_page_size)
    {
        throw runtime_error("new max page size is less than min page size");
    }
    buffer->updateMaximumSizeInPages(new_max_page_size);
}

void Db3::close() {
    lsm->save();
}

