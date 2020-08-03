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

using namespace std;
using namespace sql;

class Table {
public:
    using column_val_t = pair<string, string>;

public:
    explicit Table(string table_name);
    ~Table() = default;

    bool findByID(const string &id);
    bool execute();

    void updateColumnValue(const column_val_t &column_value);

    string getColumnValue(const string &column) const;

    void setColumnValue(const column_val_t &column_value);


private:
    void _init();

    string _concat(const map<string, string>::iterator &cl_val_node);
    string _concat(const column_val_t &cl_val);
    string _concatInsert();

private:
    string _table_name;

    mutable map<string, string> row;

    // Sql data
    mysql::MySQL_Driver *driver {nullptr};
    unique_ptr<Connection> con;
    unique_ptr<Statement> stmt;
    unique_ptr<ResultSet> sql_res;
};


#endif //SQLWRAPPER_SQLWRAPPER_H
