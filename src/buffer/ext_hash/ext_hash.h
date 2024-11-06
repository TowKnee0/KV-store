#ifndef EXT_HASH_H
#define EXT_HASH_H
#include "../buffer.h"
#include "../../../include/xxhash.hpp"
#include <optional>
#include <vector>
#include <memory>

using namespace std;
struct frame
{
    bool frame_in_use;
    vector<pair<int, int>> data;
    string filename;
    int pageNumber;
    xxh::hash_t<32> hash;
    bool dirty;
    shared_ptr<frame> next;
};

class ExtHash : public Buffer
{
private:
    // Settings for the ExtHash
    const int minimumSizeInPages;
    int maximumSizeInPages;
    int currentSizeInPages;
    const float expand_factor; // After reaching this value, double the directory size
    const int frame_size;

    int item_count;
    int overflow_count;
    vector<int> indices_with_overflow;

    int directory_bit_size;
    vector<shared_ptr<frame>> directory;
    // Directory is an array of pointers to a linked list of frames.
    void evictPage(const string filename, int pageNumber);
    void expandDirectory();
    void shrinkDirectory(int targetNumberOfPages);

    // Helpers
    shared_ptr<frame> createFrame(const string filename, int pageNumber, vector<pair<int, int>> data, bool dirty);
    shared_ptr<frame> createEmptyFrame();
    bool HandleOverflow(uint32_t overflowing_index);
    void insertFrame(shared_ptr<frame> new_frame, uint32_t index, bool check_overflow);
    uint32_t GetIndexFromHashAndBitSize(xxh::hash_t<32> hash, int bit_size);
    void checkAndExpand();
    void checkAndHandleOverflow(int overflow_value, int index);
    bool canExpand();
    int bitSizeForNumberOfPages(int numberOfPages, int frame_size);
    bool IndexIsOverflowing(int index);
    xxh::hash_t<32> getHash(const string filename, int pageNumber);

    // Temp evict function
    void EvictRandom();

public:
    ExtHash(int minimumSizeInPages, int maximumSizeInPages, float expand_factor, int frame_size);
    void updateMaximumSizeInPages(int newMaximumSizeInPages);
    void putPage(const string filename, int pageNumber, vector<pair<int, int>> data, bool dirty = false);
    optional<vector<pair<int, int>>> getPage(const string filename, int pageNumber);
};

#endif
