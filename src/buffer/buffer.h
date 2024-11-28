
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
    virtual optional<vector<pair<int, int>>> getPage(const string filename, int pageNumber) = 0;
    virtual void del(string filename, int pageNumber = -1, bool delete_file = true) = 0;
    virtual int rename(string old_name, string new_name, int pageNumber = -1) = 0;
    virtual void putPage(const string filename, int pageNumber, vector<pair<int, int>> data, bool dirty = false) = 0;
    virtual void updateMaximumSizeInPages(int newMaximumSizeInPages) = 0;
    virtual ~Buffer() = default;
};

#endif