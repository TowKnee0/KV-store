#ifndef BTREEFACTORY_H
#define BTREEFACTORY_H

#include "btree.h"

using namespace std;

class BTreeFactory {
public:
    virtual ~BTreeFactory() = default;
    virtual shared_ptr<BTree> create(string name, string dbname, shared_ptr<IO> io, shared_ptr<Buffer> buffer, shared_ptr<Filter> filter) = 0;
};

class DefaultBTreeFactory : public BTreeFactory {
public:
    shared_ptr<BTree> create(string name, string dbname, shared_ptr<IO> io, shared_ptr<Buffer> buffer, shared_ptr<Filter> filter) {
        return make_shared<BTree>(name, dbname, io, buffer, false, filter);
    }
};

#endif