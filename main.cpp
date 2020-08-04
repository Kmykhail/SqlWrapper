#include <iostream>

#include "SqlWrapper.h"

int main() {
    auto pages = Table::loadTable("Pages");
    if (pages.findByID("1")) {
        pages.printRow();
    }

    auto test = Table::loadTable("test");
    if (test.findByID("1")) {
        test.printRow();
    }

    return 0;
}