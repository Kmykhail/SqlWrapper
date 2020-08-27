//
// Created by kmykhail on 03.08.20.
//

#include "SqlWrapper.h"

unique_ptr<SqlObj> Table::_sql_obj(nullptr);

Table Table::loadTable(const string &table) {
    return Table(table);
}

Table::Table(string table) : _table_name(table) {
    if (!_sql_obj.get()) _init();
}

void Table::_init() {
    _sql_obj.reset(new SqlObj);

    auto &[driver, con, stmt, sql_res] = *_sql_obj;

    driver = mysql::get_mysql_driver_instance();

    con.reset(driver->connect("localhost:3306", "root", ""));
    con->setSchema("CB1");

    stmt.reset(con->createStatement());
}

bool Table::findByID(const string &id) {
    auto &sql_res = _sql_obj->sql_res;
    auto &stmt = _sql_obj->stmt;

    sql_res.reset(stmt->executeQuery("SELECT * FROM " + _table_name + " WHERE ID=" + id + " ORDER BY id ASC"));

    uint32_t sql_status = 0;

    ResultSetMetaData *res_meta = sql_res->getMetaData();
    uint32_t column_count = res_meta->getColumnCount();

    while(sql_res->next()) {

        for (uint32_t i = 1; i <= column_count; ++i) {
            auto [row_node, res] = _row_data.emplace(res_meta->getColumnName(i), "\'" + sql_res->getString(i) + "\'");
            if (res) {
                _order_row.emplace_back(row_node);
            }
        }

        ++sql_status;
    }

    if (sql_res) _exec_type = ExecuteType ::ReadyForUpdate;

    return sql_status;
}

string Table::getColumnValue(const string &cl) const {

    auto cl_value_node = _row_data.find(cl);

    assert((cl_value_node != _row_data.end()) && "There is no such column in the table");

    return cl_value_node->second;
}

void Table::setColumnValue(const column_val_t &column_value) {
    _row_data.emplace(column_value);

//    auto &[cl, val] = column_value;

//    auto cl_value_node = _row_data.find(cl);
//
//    assert((cl_value_node != _row_data.end()) && "There is no such column in the table");
//
//    cl_value_node->second = val;
}

void Table::updateColumnValue(const column_val_t &column_value) {
    auto &stmt = _sql_obj->stmt;

    auto cl_value_node = _row_data.find(column_value.first);

    assert((cl_value_node != _row_data.end()) && "There is no such column in the table");

    stmt->executeUpdate("UPDATE " + _table_name + " SET " + _concat(column_value));
}

bool Table::execute() {
    auto &sql_res = _sql_obj->sql_res;
    auto &stmt = _sql_obj->stmt;

    switch (_exec_type) {
        case ExecuteType::ReadyForInsert :
        {
            uint32_t sql_status = 0;

            sql_res.reset(stmt->executeQuery("INSERT INTO " + _table_name + " VALUES (" + _concatInsert() + ")"));
            while(sql_res->next()) ++sql_status;

            if (!sql_status) return false;
        }
        case ExecuteType::ReadyForUpdate :
        {
            auto id_node = _row_data.find("ID");
            if (id_node == _row_data.end()) return false;

            auto stmt_res = stmt->executeUpdate("UPDATE " + _table_name + " SET " + _concatSet() + " WHERE ID=" + id_node->second);
            if (!stmt_res) return false;

        }
        default:
            break;
    }

    return true;
}

string Table::_concatAllRows() const {
    if (_row_data.empty()) return {};

    string res;

    auto last_element = _row_data.rbegin();

    for (const auto &data : _row_data) {
        res += _concat(data);
        if (data != *last_element) res += " AND ";
    }

    return res;
}

string Table::_concat(const Table::column_val_t &cl_val) const {
    return cl_val.first + "=" + cl_val.second;
}

string Table::_concatInsert() const {

    string res;

    for (auto it = _order_row.begin(); it != _order_row.end(); ++it) {
        res += (*it)->second;
        if (it != _order_row.end() - 1) res += ",";
    }

    return res;
}

string Table::_concatSet() const {

    string res;

    for (auto it = _order_row.begin(); it != _order_row.end(); ++it) {
        res += (*it)->first + "=" + (*it)->second;
        if (it != _order_row.end() - 1) res += ",";
    }

    return res;
}

void Table::printRow() const {
    for (const auto &[column, val] : _row_data) {
        cout << column << ", " << val << endl;
    }
}

void Table::applyDBSettings() {
    auto &[driver, con, stmt, sql_res] = *_sql_obj;

    con.reset(driver->connect(_host, _user, _password));
    con->setSchema(_schema);

    stmt.reset(con->createStatement());
}


TableCollection::TableCollection(string table_name) : Table(move(table_name)) {}

void TableCollection::findArray() {
    auto &sql_res = _sql_obj->sql_res;
    auto &stmt = _sql_obj->stmt;

    sql_res.reset(stmt->executeQuery("SELECT * FROM " + _table_name + " WHERE\n" + _concatAllRows()));

    ResultSetMetaData *res_meta = sql_res->getMetaData();

    uint32_t column_count = res_meta->getColumnCount();

    while(sql_res->next()) {

        for (uint32_t i = 1; i <= column_count; ++i) {
            auto [row_node, res] = _row_data.emplace(res_meta->getColumnName(i), "\'" + sql_res->getString(i) + "\'");
            if (res) {
                _order_row.emplace_back(row_node);
            }
        }

        _table_rows.emplace_back(move(static_cast<Table&>(*this)));

        _row_data.clear();
        _order_row.clear();
    }
}
