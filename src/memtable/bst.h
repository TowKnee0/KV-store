#ifndef BST_H
#define BST_H

#include <optional>

using namespace std;

enum colors {RED, BLACK};
enum ErrorCode {SUCCESS, TREE_FULL};

struct node
    {
        int key;
        int value;
        colors color;
        node* left;
        node* right;
        node* parent;
};

class Bst {
    private:
        int max_size;
        int c_size;
        node* root;

        void leftRotate(node* x);
        void rightRotate(node * x);

        void fix(node* z);
    public:
        Bst(int memtable_size);
        ErrorCode put(int key, int value);
        optional<int> get(int key);
};

#endif