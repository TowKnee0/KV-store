#include "bst.h"
#include <optional>
#include <iostream>

using namespace std;

// implementations are taken directly from CLRS 
Bst::Bst(int memtable_size) {
    max_size = memtable_size;
    c_size = 0;
}

void Bst::leftRotate(node* x) {
    if (x == nullptr || x->right == nullptr)
        return;

    node* y  = x->right;
    x->right = y->left;

    if (y->left != nullptr){
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr){
        root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

void Bst::rightRotate(node* y) {
    if (y == nullptr || y->left == nullptr)
        return;


    node* x = y->left;
    y->left = x->right;
    if (x->right != nullptr){
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == nullptr){
        root = x;
    } else if (y == y->parent->right){
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }
    x->right = y;
    y->parent = x;
}

ErrorCode Bst::put(int key, int value) {
    if (c_size == max_size) {
        return TREE_FULL;
    }

    node* x = root;
    node* y = nullptr;
    node* z = new node();
    z->key = key;
    z->value = value;

    while (x != nullptr) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == nullptr) {
        root = z;
    } else if (z->key < y->key){
        y->left = z;
    } else {
        y->right = z;
    }
    z->left = nullptr;
    z->right = nullptr;
    z->color = RED;
    fix(z);
    c_size++;
    cout << "GOT HERE3";
    return SUCCESS;
}

void Bst::fix(node* z) {
    while (z!= root && z->parent->color == RED){
        if (z->parent == z->parent->parent->left){
            node* y = z->parent->parent->right;
            if (y != nullptr && y->color == RED) {
                z->parent->color == BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else{
                if (z == z->parent->right){
                    z = z->parent;
                    leftRotate(z);
                }
                z->parent->color == BLACK;
                z->parent->parent->color = RED;
                rightRotate(z->parent->parent);
            }
        } else {
            node* y = z->parent->parent->left;
            if (y != nullptr && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left){
                    z = z->parent;
                    rightRotate(z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                leftRotate(z->parent->parent);
            }
        }
    }
    root->color = BLACK;
}

optional<int> Bst::get(int key){
    node* curr = root;
    while (curr != nullptr){
        if (key == curr->key){
            return curr->value;
        } else if (key < curr->key){
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return nullopt;
}
