#include <iostream>

#include "SqlWrapper.h"

int main() {
    Table sw("Pages");

    sw.findByID("1");
    sw.updateColumnValue({"LangID", "12"});

    return 0;
}