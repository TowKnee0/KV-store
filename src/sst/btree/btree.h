#ifndef BTree_H
#define BTree_H

#include <optional>
#include <vector>
#include <string>
#include <memory>
#include "../../io/io.h"
#include "../sst.h"
#include "../../buffer/buffer.h"
#include "../../filter/filter.h"
#include <limits>

using namespace std;

class BTree : public Sst
{
private:
    string dbname;
    shared_ptr<IO> io_handler;
    shared_ptr<Buffer> buffer;
    // Filters are optional as part 2 doesn't have them.
    shared_ptr<Filter> filter;
    int B;
    bool use_binary_search = false;

    // Metadata. Metadata is stored in the last page of the file.
    // You can get the last page by calling io_handler->numPagesInFile(filename)
    static constexpr int B_TREE_SIZE_INDEX = 0;
    static constexpr int B_TREE_HEIGHT_INDEX = 1;
    static constexpr int B_TREE_NUM_DATA_INDEX = 2;

    // Helpers
    int num_leaf_pages = 0;
    int num_data = 0;
    static constexpr int INF = numeric_limits<int>::max();
    vector<pair<int, int>> internal_nodes;

    // Varaibles which need to be found
    int height = -1;

    vector<pair<int, int>> contructInternalNodes();
    pair<int, pair<int, int>> findKeyInPage(int key, int page_location);
    void loadMetadata();
    vector<pair<int, int>> constructMetadata();

    optional<int> btreeGet(int key);

    // Methods for binary search
    optional<pair<vector<pair<int, int>>, int>> findPageWithValueEqualToOrGraterThan(int key);
    optional<pair<pair<int, int>, int>> FindEqualToOrNextLargestInPage(int key, vector<pair<int, int>> page);
    optional<int> binarySearchGet(int key);

public:
    string filename; // Which file the btree is stored in

    BTree(string filename, string dbname, shared_ptr<IO> io_handler, shared_ptr<Buffer> buffer, bool use_binary_search = false, shared_ptr<Filter> filter = nullptr);
    virtual void rename(string filename);
    virtual int write(const vector<pair<int, int>> &data);
    virtual optional<int> get(int key); // Search the corresponding value for this key.
    virtual int mergeBTrees(string filename1, string filename2);
    virtual vector<int> getKeyData(int key);
    virtual string getFilename();
    virtual int getNumDataPages();
};

#endif