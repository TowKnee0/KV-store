#ifndef BST_H
#define BST_H

#include <optional>
#include <memory>

using namespace std;

enum colors {RED, BLACK};
enum ErrorCode {SUCCESS, TREE_FULL};

struct node
    {
        int key;
        int value;
        colors color;
        shared_ptr<node> left;
        shared_ptr<node> right;
        weak_ptr<node> parent;
};

class Bst {
    private:
        int max_size;
        int c_size;
        shared_ptr<node> root;

        void leftRotate(shared_ptr<node> x);
        void rightRotate(shared_ptr<node> x);

        void fix(shared_ptr<node> z);

        ~Bst();
        void clear(shared_ptr<node> n);
    public:
        Bst(int memtable_size);
        ErrorCode put(int key, int value);
        optional<int> get(int key);
};

#endif