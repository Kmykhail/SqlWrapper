//
// Created by kmykhail on 03.08.20.
//

#include "SqlWrapper.h"

unique_ptr<SqlObj> Table::sql_obj(nullptr);

Table Table::loadTable(const string &table) {
    return Table(table);
}

Table::Table(string table) : _table_name(table) {
    if (!sql_obj.get()) _init();
}

void Table::_init() {
    sql_obj.reset(new SqlObj);

    auto &[driver, con, stmt, sql_res] = *sql_obj;

    driver = mysql::get_mysql_driver_instance();

    con.reset(driver->connect("localhost:3306", "root", ""));
    con->setSchema("CB1");

    stmt.reset(con->createStatement());
}

bool Table::findByID(const string &id) {
    auto &sql_res = sql_obj->sql_res;
    auto &stmt = sql_obj->stmt;

    sql_res.reset(stmt->executeQuery("SELECT * FROM " + _table_name + " WHERE ID=" + id + " ORDER BY id ASC"));

    uint32_t sql_status = 0;

    ResultSetMetaData *res_meta = sql_res->getMetaData();
    uint32_t column_count = res_meta->getColumnCount();

    while(sql_res->next()) {

        for (uint32_t i = 1; i <= column_count; ++i) {
            row.emplace(res_meta->getColumnName(i), sql_res->getString(i));
        }

        ++sql_status;
    }

    return sql_status;
}

string Table::getColumnValue(const string &cl) const {

    auto cl_value_node = row.find(cl);

    assert((cl_value_node != row.end()) && "There is no such column in the table");

    return cl_value_node->second;
}

void Table::setColumnValue(const column_val_t &column_value) {

    auto &[cl, val] = column_value;

    auto cl_value_node = row.find(cl);

    assert((cl_value_node != row.end()) && "There is no such column in the table");

    cl_value_node->second = val;

}

void Table::updateColumnValue(const column_val_t &column_value) {
    auto &stmt = sql_obj->stmt;

    auto cl_value_node = row.find(column_value.first);

    assert((cl_value_node != row.end()) && "There is no such column in the table");

    stmt->executeUpdate("UPDATE " + _table_name + " SET " + _concat(column_value));
}

// TODO
bool Table::execute() {
//    sql_res.reset(stmt->executeQuery("INSERT INTO " + _table_name + " VALUES (" + _concatInsert() + ")"));
    return false;
}

string Table::_concat(const map<string, string>::iterator &cl_val_node) {
    return cl_val_node->first + "=" + cl_val_node->second;
}

string Table::_concat(const Table::column_val_t &cl_val) {
    return cl_val.first + "=" + cl_val.second;
}

// TODO
string Table::_concatInsert() {

}

void Table::printRow() const {
    for (const auto &[column, val] : row) {
        cout << column << ", " << val << endl;
    }
}

void Table::applyDBSettings() {
    auto &[driver, con, stmt, sql_res] = *sql_obj;

    con.reset(driver->connect(_host, _user, _password));
    con->setSchema(_schema);

    stmt.reset(con->createStatement());
}

