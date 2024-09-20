#ifndef BST_H
#define BST_H

class Bst {
    struct node
{
    int key;
    int value;
    node* left;
    node* right;
};


    private:
        int memtable_size;
        node* root;

    public:
        Bst(int memtable_size);
        void put(int key, int value);
        int get(int key);
};

#endif