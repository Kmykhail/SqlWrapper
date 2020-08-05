#include <iostream>
#include <sstream>
#include <iterator>

#include "SqlWrapper.h"

int main() {
    auto pages = Table::loadTable("Pages");
    if (pages.findByID("1")) {
        pages.printRow();
    }

    pages.setColumnValue({"SiteID", "957"});
    pages.execute();

    return 0;
}