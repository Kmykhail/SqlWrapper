//
// Created by kmykhail on 03.08.20.
//

#include "SqlWrapper.h"

BaseTable::BaseTable(const string &table_name, const shared_ptr<SqlObj> &sql, bool check_availability) :_table_name(table_name), _sql(sql)
{
    if (check_availability) {
        auto &sql_res = _sql->sql_res;
        auto &stmt = _sql->stmt;

        sql_res.reset(stmt->executeQuery("SELECT * FROM INFORMATION_SCHEMA.TABLES "
                                         "WHERE TABLE_NAME=" + string("'" + _table_name + "'")));

        uint32_t sql_status = 0;
        while (sql_res->next()) ++sql_status;

        assert((sql_status) && "There is no such table");
    }
}

string BaseTable::_concat(const BaseTable::column_val_t &cl_val) const {
    return cl_val.first + "=" + cl_val.second;
}

string BaseTable::_concatAllRows() const {
    string res;

    auto last_element = _row_entry.column_value.rbegin();
    for (const auto &it : _row_entry.column_value) {
        res += _concat(it);
        if (it != *last_element) res += " AND ";
    }

    return res;
}

string BaseTable::_concatInsert() const {
    string res = " (";

    string values;

    const auto &column_value = _row_entry.column_value;

    for (auto it = column_value.begin(); it != column_value.end(); ++it) {
        res += it->first;
        values += it->second;
        if (it != column_value.end() - 1) {
            res += ",";
            values += ",";
        }
    }

    res += ") ";
    res += "VALUES (" + move(values) + ")";

    return res;
}

string BaseTable::_concatSet() const {
    string res;

    const auto &column_value = _row_entry.column_value;

    for (auto it = column_value.begin(); it != column_value.end(); ++it) {

        {
            string val = it->second;
            val.erase(remove(val.begin(), val.end(), '\''), val.end());
            if (val.empty()) continue;
        }

        res += it->first + "=" + it->second;
        if (it != column_value.end() - 1) res += ",";
    }

    return res;
}

void BaseTable::printEntry() const {
    for (const auto &[clm, val] : _row_entry.column_value) {
        cout << clm << ": " << val << endl;
    }
}

void BaseTable::truncate() {
    _sql->stmt->executeUpdate("TRUNCATE " + _table_name);
}

DBConnection DBConnection::loadDB(const string &host,
                           const string &user,
                           const string &pass,
                           const string &schema)
{
    return DBConnection(host, user, pass, schema);
}

DBConnection::DBConnection(string host,
                           string user,
                           string pass,
                           string schema) : _host(move(host)), _user(move(user)), _password(move(pass)), _schema(move(schema))
{
    if (!_sql_obj.get()) _sql_obj.reset(new SqlObj);

    auto &[driver, con, pstmt ,stmt, sql_res] = *_sql_obj;

    driver = mysql::get_mysql_driver_instance();

    con.reset(driver->connect(_host, _user, _password));
    con->setSchema(_schema);

    stmt.reset(con->createStatement());

    _is_success_conn = true;
}

TableCollection DBConnection::getTableCollection(const string &table_name) {
    return TableCollection(table_name, _sql_obj);
}

SingleRowTable DBConnection::getSingleRowTable(const string &table_name) {
    return SingleRowTable(table_name, _sql_obj);
}

SingleRowTable::SingleRowTable(const string &table_name, const shared_ptr<SqlObj> &sql, bool check_availability) : BaseTable(table_name, sql, check_availability)
{
}

//bool SingleRowTable::findByColumnValue(const BaseTable::column_val_t &cl_val) {
//    _row_entry.column_value.clear();
//    _row_entry.column_value_iter.clear();
//
//    auto &sql_res = _sql->sql_res;
//    auto &stmt = _sql->stmt;
//
//    sql_res.reset(stmt->executeQuery("SELECT * FROM " + _table_name + " WHERE " + _concat(cl_val) + " ORDER BY id ASC"));
//
//    auto status = _getQueryData(sql_res);
//    if (status) _exec_type = ExecuteType ::ReadyForUpdate;
//
//    return status;
//}

bool SingleRowTable::findByID(const string &id) {
    _row_entry.clear();

    auto &sql_res = _sql->sql_res;
    auto &stmt = _sql->stmt;

    sql_res.reset(stmt->executeQuery("SELECT * FROM " + _table_name + " WHERE ID=" + id + " ORDER BY id ASC"));

    auto status = _getQueryData(sql_res);
    if (status) _exec_type = ExecuteType ::ReadyForUpdate;

    return status;
}

bool SingleRowTable::_getQueryData(const shared_ptr<ResultSet> &sql_res) {
    uint32_t sql_status = 0;

    ResultSetMetaData *res_meta = sql_res->getMetaData();
    auto count = res_meta->getColumnCount();

    while(sql_res->next()) {

        for (auto i = 1; i <= count; ++i) {
            _row_entry.emplace(res_meta->getColumnName(i), "\'" + sql_res->getString(i) + "\'");
        }
        ++sql_status;
    }

    return sql_status;
}

void SingleRowTable::_fetchRow(shared_ptr<ResultSet> &sql_res, shared_ptr<Statement> &stmt) {
    sql_res.reset(stmt->executeQuery("SELECT LAST_INSERT_ID() FROM " + _table_name));

    while(sql_res->next()) {
        findByID(sql_res->getString(1));
    }
}

bool SingleRowTable::execute() {
    if (_row_entry.empty()) return false;

    auto &sql_res = _sql->sql_res;
    auto &stmt = _sql->stmt;

    bool sql_status  {false};

    switch (_exec_type) {
        case ExecuteType::ReadyForInsert :
        {
            sql_status = stmt->executeUpdate("INSERT INTO " + _table_name + _concatInsert());
            if (sql_status) _fetchRow(sql_res, stmt);

            break;
        }
        case ExecuteType::ReadyForUpdate :
        {
            auto id_node = _row_entry.find("ID");
            if (id_node == _row_entry.end()) return false;

            sql_status = stmt->executeUpdate("UPDATE " + _table_name + " SET " + _concatSet() + " WHERE ID=" + id_node->second);
        }
        default:
            break;
    }

    return sql_status;
}

string SingleRowTable::getValueByColumn(const string &column) const{
    auto column_value_node = _row_entry.find(column);
    if (column_value_node == _row_entry.column_value.end()) return {};

    string value = column_value_node->second;

    value.erase(remove(value.begin(), value.end(), '\''), value.end());

    return value;
}

void SingleRowTable::setColumnValue(column_val_t column_value) {
    auto &[clm, val] = column_value;
    val = "\'" + val + "\'";

    auto row_entry_node = _row_entry.find(clm);

    switch (_exec_type) {
        case ExecuteType::ReadyForUpdate : {
            assert((row_entry_node != _row_entry.end()) && "There is no such column in the table");

            row_entry_node->second = val;

            break;
        }
        case ExecuteType::ReadyForInsert :
        {
            if (row_entry_node != _row_entry.end()) {
                row_entry_node->second = val;
            } else {
                _row_entry.emplace(column_value);
            }

            break;
        }
        default:
            break;
    }
}

void SingleRowTable::updateColumnValue(column_val_t column_value) {
    auto &[clm, val] = column_value;
    val = "\'" + val + "\'";

    auto &sql_res = _sql->sql_res;
    auto &stmt = _sql->stmt;

    auto id_node = _row_entry.find("ID");
    assert((id_node != _row_entry.end()) && "Сan't update value without ID");

    auto row_entry_node = _row_entry.find(column_value.first);
    assert((row_entry_node != _row_entry.end()) && "There is no such column in the table");

    stmt->executeUpdate("UPDATE " + _table_name + " SET " + _concat(column_value) + " WHERE ID=" + id_node->second);
    row_entry_node->second = val;
}

string SingleRowTable::getID() const {
    if (_row_entry.empty()) return {};

    auto id_node = _row_entry.find("ID");

    if (id_node == _row_entry.end()) return {};
    return id_node->second;
}

void SingleRowTable::reset() {
    _row_entry.clear();
    _exec_type = ExecuteType::ReadyForInsert;
}

TableCollection::TableCollection(const string &table_name, const shared_ptr<SqlObj> &sql) : BaseTable(table_name, sql)
{
}

string TableCollection::_concatAllRows() const {
    string res;

    auto last_element = _clm_value.rbegin();
    for (const auto &it : _clm_value) {
        res += _concat(it);
        if (it != *last_element) res += " AND ";
    }

    return res;
}

void TableCollection::findArray() {
    auto &sql_res = _sql->sql_res;
    auto &stmt = _sql->stmt;

    const string where_statement = _concatAllRows();

    if (where_statement.empty()) {
        sql_res.reset(stmt->executeQuery("SELECT * FROM " + _table_name));
    } else {
        sql_res.reset(stmt->executeQuery("SELECT * FROM " + _table_name + " WHERE\n" + _concatAllRows()));
    }

    auto status = _getQueryData(sql_res);

    _clm_value.clear();
}

bool TableCollection::_getQueryData(const shared_ptr<ResultSet> &sql_res) {
    bool sql_status {false};

    ResultSetMetaData *res_meta = sql_res->getMetaData();

    auto count = res_meta->getColumnCount();

    while (sql_res->next()) {
        _collection.emplace_back(_table_name, _sql, false);

        auto &single_row = _collection.back();

        for (auto i = 1; i <= count; ++i) {
            single_row.setColumnValue({res_meta->getColumnName(i), sql_res->getString(i)});
        }

        single_row.setExecuteType(ExecuteType::ReadyForUpdate);

        sql_status = true;
    }
    return sql_status;
}

void TableCollection::setColumnValue(column_val_t column_value) {
    auto &[clm, val] = column_value;
    val = "\'" + val + "\'";

    _clm_value[clm] = val;
}

size_t TableCollection::getCount() const {
    return _collection.size();
}
