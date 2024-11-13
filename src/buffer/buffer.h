
#ifndef Buffer_H
#define Buffer_H
using namespace std;
#include <string>
#include <vector>
#include <optional>

class Buffer
{
private:
    int minimumSizeInPages;

public:
    virtual optional<vector<pair<int, int>>> getPage(const string filename, int pageNumber, bool get_from_storage = true) = 0;
    virtual void putPage(const string filename, int pageNumber, vector<pair<int, int>> data, bool dirty = false) = 0;
    virtual void updateMaximumSizeInPages(int newMaximumSizeInPages) = 0;
    virtual ~Buffer() = default;
};

#endif