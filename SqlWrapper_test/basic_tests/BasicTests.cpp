//
// Created by mykhailenko on 03.09.20.
//

#include <gtest/gtest.h>
#include "SqlWrapper.h"
#include <random>

struct TableTest : testing::Test {
    DBConnection db_obj = DBConnection::loadDB("localhost:3306", "root", "root", "CB1");
};

TEST_F(TableTest, TestDBConnection) {
    EXPECT_EQ(db_obj.isSuccessConn(), true);
}

TEST_F(TableTest, TotalSingleRowTableTest) {
    SingleRowTable single_row = db_obj.getSingleRowTable("Pages");

    single_row.truncate();

    EXPECT_EQ(single_row.execute(), false); // dumb check

    // record row
    single_row.setColumnValue({"SiteID", "333"});
    single_row.setColumnValue({"Url", "http://test-url"});
    EXPECT_EQ(single_row.execute(), true);

    auto id = single_row.getID();
    EXPECT_TRUE(!id.empty());

    single_row.reset();

    // find by id
    EXPECT_TRUE(single_row.findByID(id));
    EXPECT_EQ(single_row.getValueByColumn("SiteID"), "333");
    EXPECT_EQ(single_row.getValueByColumn("Url"), "http://test-url");

    // update column
    single_row.updateColumnValue({"Url", "http://new-test-url"});
    EXPECT_EQ(single_row.getValueByColumn("Url"), "http://new-test-url");

    single_row.reset();

    // record row
    single_row.setColumnValue({"SiteID", "117745"});
    single_row.setColumnValue({"Url", "http://some-url"});
    EXPECT_EQ(single_row.execute(), true);

    single_row.reset();
    // record row
    single_row.setColumnValue({"SiteID", "117745"});
    single_row.setColumnValue({"Url", "http://some-another-url"});
    EXPECT_EQ(single_row.execute(), true);
}

TEST_F(TableTest, TotalTableCollectionTest) {
    TableCollection tab_coll = db_obj.getTableCollection("Pages");

    tab_coll.setColumnValue({"SiteID", "117745"});
    tab_coll.findArray();
    EXPECT_EQ(tab_coll.getCount(), 2);

    for (auto &single_row : tab_coll) {
        EXPECT_EQ(single_row.getValueByColumn("SiteID"), "117745");
        if (single_row.getValueByColumn("Url") == "http://some-another-url") {
            single_row.setColumnValue({"Url", "http://some-another-url-changed"});
        } else if (single_row.getValueByColumn("Url") == "http://some-url") {
            single_row.setColumnValue({"Url", "http://some-url-changed"});
        }

        EXPECT_EQ(single_row.execute(), true);
    }
}

int main(int argc, char **argv) {
    cout << "!!!!!!!!!!!" << endl;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}