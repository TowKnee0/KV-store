#include "db3.h"
#include <iostream>

using namespace std;

int a, b;
string query;
optional<int> val;

int main()
{
    /* 
    Change these to configure db.

    Note that each time these values are changed, a new dbname should be used otherwise there may be undefined behaviour.
    */
    const int entry_size = 16;
    const int memtable_size = 1048576 / entry_size;
    const int min_buffer_pages = 1;
    const int max_buffer_pages = 1000;
    const float buffer_expand_factor = 0.75;
    const int max_buffer_chain_length = 10;
    const int bloom_filter_bits = 8;
    const string dbname = "test";

    unique_ptr<Db> db = make_unique<Db3>(2, min_buffer_pages, max_buffer_pages, buffer_expand_factor, max_buffer_chain_length, bloom_filter_bits, false);

    db->open(dbname);
    cout << "db opened" << endl;

    while (true) {
        cin >> query;

        if (query == "open") {
            cout << "DB is already open. Please close before opening another DB." << endl;
        } else if (query == "put") {
            cin >> a >> b;

            db->put(a, b);
        } else if (query == "get") {
            cin >> a;
            val = db->get(a);
            if (val.has_value()) {
                cout << val.value() << endl;
            } else {
                cout << "null" << endl;
            }
        } else if (query == "scan") {
            cin >> a >> b;
            vector<pair<int, int>> res = db->scan(a, b);
            for (int i = 0; i < res.size(); i++) {
                cout << "(" << res[i].first << ", " << res[i].second << "), ";
            }
            cout << endl;
        } else if (query == "del") {
            cin >> a;
            db->del(a);
        } else if (query == "close") {
            break;
        } else {
            cout << "Invalid query request" << endl;
        }
    }

    db->close();
    cout << "db closed" << endl;
}