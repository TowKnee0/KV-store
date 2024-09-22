#include "bst.h"
#include <optional>
#include <iostream>
#include <memory>

using namespace std;

// implementations were taken directly from CLRS pseudocode.
// only addition made is the use for smart pointers.
Bst::Bst(int memtable_size) : max_size(memtable_size), c_size(0), root(nullptr) {}

void Bst::leftRotate(shared_ptr<node> x) {
    if (x == nullptr || x->right == nullptr)
        return;

    shared_ptr<node> y = x->right;
    x->right = y->left;

    if (y->left != nullptr) {
        y->left->parent = x;
    }
    y->parent = x->parent;

    shared_ptr<node> xp = x->parent.lock();
    if (xp == nullptr) {
        root = y;
    } else if (x == xp->left)
    {
        xp->left = y;
    } else {
        xp->right = y;
    }
    
    y->left = x;
    x->parent = y;
}

void Bst::rightRotate(shared_ptr<node> y) {
    if (y == nullptr || y->left == nullptr)
        return;

    shared_ptr<node> x = y->left;
    y->left = x->right;

    if (x->right != nullptr) {
        x->right->parent = y;
    }
    x->parent = y->parent;

    shared_ptr<node> yp = y->parent.lock();
    if (yp == nullptr) {
        root = x;
    } else if (y == yp->right) {
        yp->right = x;
    } else {
        yp->left = x;
    }

    x->right = y;
    y->parent = x;
}

ErrorCode Bst::put(int key, int value) {
    if (c_size == max_size) {
        return TREE_FULL;
    }

    shared_ptr<node> x = root;
    shared_ptr<node> y = nullptr;
    shared_ptr<node> z = make_shared<node>();
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
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }
    z->left = nullptr;
    z->right = nullptr;
    z->color = RED;
    fix(z);
    c_size++;
    return SUCCESS;
}

void Bst::fix(shared_ptr<node> z) {
    while (z != root && z->parent.lock()->color == RED) {
        if (z->parent.lock() == z->parent.lock()->parent.lock()->left) {
            shared_ptr<node> y = z->parent.lock()->parent.lock()->right;
            if (y != nullptr && y->color == RED) {
                z->parent.lock()->color = BLACK;
                y->color = BLACK;
                z->parent.lock()->parent.lock()->color = RED;
                z = z->parent.lock()->parent.lock();
            } else {
                if (z == z->parent.lock()->right) {
                    z = z->parent.lock();
                    leftRotate(z);
                }
                z->parent.lock()->color = BLACK;
                z->parent.lock()->parent.lock()->color = RED;
                rightRotate(z->parent.lock()->parent.lock());
            }
        } else {
            shared_ptr<node> y = z->parent.lock()->parent.lock()->left;
            if (y && y->color == RED) {
                z->parent.lock()->color = BLACK;
                y->color = BLACK;
                z->parent.lock()->parent.lock()->color = RED;
                z = z->parent.lock()->parent.lock();
            } else {
                if (z == z->parent.lock()->left) {
                    z = z->parent.lock();
                    rightRotate(z);
                }
                z->parent.lock()->color = BLACK;
                z->parent.lock()->parent.lock()->color = RED;
                leftRotate(z->parent.lock()->parent.lock());
            }
        }
    }
    root->color = BLACK;
}

optional<int> Bst::get(int key) {
    shared_ptr<node> curr = root;
    while (curr != nullptr) {
        if (key == curr->key) {
            return curr->value;
        } else if (key < curr->key) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return nullopt;
}

vector<pair<int, int>> Bst::inOrder() {
    vector<pair<int, int>> v; 

    inOrderHelper(root, v);
    return v;
}

void Bst::inOrderHelper(shared_ptr<node> c, vector<pair<int, int>>& v) {
    if (c == nullptr) {
        return;
    }

    inOrderHelper(c->left, v);
    v.push_back(make_pair(c->key, c->value));
    inOrderHelper(c->right, v);
}

void Bst::clear() {
    root = nullptr;
    c_size = 0;
}