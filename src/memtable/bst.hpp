#ifndef BST_H
#define BST_H

class Bst {
    private:
        int max_size;
        int c_size;
        node* root;
        node* sentinel; // TODO: denotes nil. check if this is needed

        void leftRotate(node* x);
        void rightRotate(node * x);

        void fix(node* z);
    public:
        Bst(int memtable_size);
        void put(int key, int value);
        int get(int key);
};

enum color {RED, BLACK};

struct node
    {
        int key;
        int value;
        color color;
        node* left;
        node* right;
        node* parent;
};

#endif