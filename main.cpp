#include <iostream>
#include <sstream>
#include <iterator>

#include "SqlWrapper.h"

int main() {
    auto pages = Table::loadTable("Pages");
    if (pages.findByID("1")) {
        pages.printRow();
    }

    pages.setColumnValue({"SiteID", "222"});
    pages.execute();

    TableCollection tab_coll("TestCB_1");
    tab_coll.setColumnValue({"SiteID", "12"});
    tab_coll.findArray();

    cout << "!!!!!!Check FindArray!!!!!" << endl;

    for (auto &it : tab_coll) {
        it.printRow();
    }


    return 0;
}