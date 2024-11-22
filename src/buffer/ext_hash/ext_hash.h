#ifndef EXT_HASH_H
#define EXT_HASH_H
#include "../buffer.h"
#include "../../io/io.h"
#include "../../../include/xxhash.hpp"
#include <optional>
#include <vector>
#include <memory>

using namespace std;
struct frame
{
    bool recently_used;
    vector<pair<int, int>> data;
    string filename;
    int pageNumber;
    xxh::hash_t<32> hash;
    bool dirty;
    bool deleted;
    shared_ptr<frame> next;
};

class ExtHash : public Buffer
{
private:
    // Settings for the ExtHash
    const int minimumSizeInPages;
    int maximumSizeInPages;
    const float expand_factor; // After reaching this value, double the directory size
    const int frame_size;
    shared_ptr<IO> io_handler;

    int item_count;
    vector<int> bucket_counts;


    void checkAndSplitBucket(int index);

    vector<shared_ptr<frame>> clock;
    int curr_hand;

    int directory_bit_size;
    vector<shared_ptr<frame>> directory;
    // Directory is an array of pointers to a linked list of frames.
    void shrinkDirectory(int targetNumberOfPages);

    // Helpers
    shared_ptr<frame> createFrame(const string filename, int pageNumber, vector<pair<int, int>> data, bool dirty);
    uint32_t GetIndexFromHashAndBitSize(xxh::hash_t<32> hash, int bit_size);
    void checkAndExpand();
    int bitSizeForNumberOfPages(int numberOfPages, int frame_size);
    xxh::hash_t<32> getHash(const string filename, int pageNumber);

    // Temp evict function
    void evict();

public:
    ExtHash(int minimumSizeInPages, int maximumSizeInPages, float expand_factor, int frame_size, shared_ptr<IO> io_handler);
    void updateMaximumSizeInPages(int newMaximumSizeInPages);
    void del(string filename, int pageNumber = -1);
    void putPage(const string filename, int pageNumber, vector<pair<int, int>> data, bool dirty = false);
    optional<vector<pair<int, int>>> getPage(const string filename, int pageNumber);
};

#endif
