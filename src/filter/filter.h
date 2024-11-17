#ifndef FILTER_H
#define FILTER_H

using namespace std;

class Filter {
    public: 
        virtual ~Filter() = default;
        virtual bool hasKey(int key) = 0;
        virtual void addKey(int key) = 0;
};

#endif