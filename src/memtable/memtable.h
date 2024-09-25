#ifndef Memtable_H
#define Memtable_H

#include <memory>
#include <vector>
#include <optional>

using namespace std;

class Memtable {
    public:
        virtual int put(int key, int value)=0;
        virtual optional<int> get(int key)=0;
        virtual vector<pair<int, int>> inOrder()=0;
        virtual int count()=0;
        virtual void clear()=0;

        virtual ~Memtable() = default;

};

#endif