#include <string>
#include <optional>
#include <vector>

using namespace std;

class Db{
public:
    virtual void open(string name)=0;
    virtual void put(int key, int value)=0;
    virtual optional<int> get(int key)=0;
    virtual vector<pair<int, int>> scan(int key1, int key2)=0;
    virtual void close()=0;

    virtual ~Db() = default;
};
