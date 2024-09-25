#ifndef BST_H
#define BST_H

#include <optional>
#include <memory>
#include <vector>
#include <utility>
#include "../memtable.h"

using namespace std;

enum colors {RED, BLACK};

struct node
    {
        int key;
        int value;
        colors color;
        shared_ptr<node> left;
        shared_ptr<node> right;
        weak_ptr<node> parent;
};

class Bst: public Memtable {
    private:
        int max_size;
        int c_size;
        shared_ptr<node> root;

        void leftRotate(shared_ptr<node> x);
        void rightRotate(shared_ptr<node> x);
        void fix(shared_ptr<node> z);

        void inOrderHelper(shared_ptr<node> c, vector<pair<int, int>>& v);
    public:
        Bst(int memtable_size);

        int put(int key, int value);
        optional<int> get(int key);

        vector<pair<int, int>> inOrder();

        int count();
        void clear();
};

#endif