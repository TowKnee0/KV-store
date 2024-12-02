#include "db1.h"
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
    const string dbname = "test";

    unique_ptr<Db> db = make_unique<Db1>(memtable_size);

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
            cout << "del not implemented in STEP 1" << endl;
        } else if (query == "close") {
            break;
        } else {
            cout << "Invalid query request" << endl;
        }
    }

    db->close();
    cout << "db closed" << endl;
}