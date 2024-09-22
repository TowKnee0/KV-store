#include <string>
#include <optional>
#include <vector>

using namespace std;

class Db{
public:
    virtual void open(string name) const = 0;
    virtual void put(int key, int value) const = 0;
    virtual optional<int> get(int key) const = 0;
    virtual vector<pair<int, int>> scan(int key1, int key2) const = 0;
    virtual void close() const = 0;

    virtual ~Db() = default;
};
