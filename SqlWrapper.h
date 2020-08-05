//
// Created by kmykhail on 03.08.20.
//

#ifndef SQLWRAPPER_SQLWRAPPER_H
#define SQLWRAPPER_SQLWRAPPER_H

#if __cplusplus >= 201703L
    /* MySQL override. This needed to be inclided before cppconn/exception.h to define them */
    #include <stdexcept>
    #include <string>
    #include <memory>

    /* Now remove the trow */
    #define throw(...)
    #include <cppconn/exception.h>
    #undef throw /* reset */

    #include <mysql_connection.h>
    #include <mysql_driver.h>
    #include <mysql_error.h>

#endif

#include <cppconn/statement.h>

#include <string>
#include <memory>
#include <list>
#include <cassert>
#include <sstream>
#include <iterator>

using namespace std;
using namespace sql;

struct SqlObj{
    mysql::MySQL_Driver *driver {nullptr};
    unique_ptr<Connection> con;
    unique_ptr<Statement> stmt;
    unique_ptr<ResultSet> sql_res;
};

enum class ExecuteType {
    NotReady,
    ReadyForInsert,
    ReadyForUpdate
};

class Table {
public:
    using column_val_t = pair<string, string>;

public:
    void setHost(const string &host) {_host = host;}
    void setUser(const string &user) {_user = user;}
    void setPassword(const string &password) {_password = password;}
    void setSchema(const string &schema) {_schema = schema;}
    void applyDBSettings();

    ~Table() = default;

    static Table loadTable(const string &table);

    bool findByID(const string &id);
    bool execute();

    void printRow() const;

    void updateColumnValue(const column_val_t &column_value);

    string getColumnValue(const string &column) const;

    void setColumnValue(const column_val_t &column_value);

private:
    explicit Table(string table);

    void _init();

    string _concat(const map<string, string>::iterator &cl_val_node) const;
    string _concat(const column_val_t &cl_val) const;

    string _concatInsert() const;
    string _concatSet() const;

private:
    string _table_name;

    vector<map<string, string>::iterator> _order_row;

    mutable map<string, string> _row;

    ExecuteType _exec_type {ExecuteType::NotReady};

    // DB settings
    string _host;
    string _user;
    string _password;
    string _schema;

    // Sql data
    static unique_ptr<SqlObj> _sql_obj;
};


#endif //SQLWRAPPER_SQLWRAPPER_H
