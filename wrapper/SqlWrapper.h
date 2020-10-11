//
// Created by kmykhail on 03.08.20.
//

#ifndef SQLWRAPPER_SQLWRAPPER_H
#define SQLWRAPPER_SQLWRAPPER_H

#if __cplusplus >= 201703L
    /* MySQL override. This needed to be included before cppconn/exception.h to define them */
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
#include <cppconn/prepared_statement.h>

#include <string>
#include <memory>
#include <list>
#include <cassert>
#include <sstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;
using namespace sql;

// TODO
// clean code
// more tests

enum class ExecuteType {
    NotReady,
    ReadyForInsert,
    ReadyForUpdate
};

struct SqlObj {
    mysql::MySQL_Driver *driver {nullptr};
    shared_ptr<Connection> con;
    shared_ptr<sql::PreparedStatement> pstmt;
    shared_ptr<Statement> stmt;
    shared_ptr<ResultSet> sql_res;
};

struct RowEntry {
    using iterator = vector<pair<string, string>>::iterator;
    using const_iterator = vector<pair<string, string>>::const_iterator;

    iterator begin() { return column_value.begin(); }
    const_iterator begin() const { return column_value.begin(); }

    iterator end() { return column_value.end(); }
    const_iterator end() const { return column_value.end(); }

    void clear() {
        column_value.clear();
    }

    bool empty() const {
        return column_value.empty();
    }

    void emplace(pair<string, string> clm_val) {
        column_value.emplace_back(clm_val);
    }

    void emplace(string clm, string val) {
        column_value.emplace_back(clm, val);
    }

    iterator find(const string &key) {
        for (auto it = column_value.begin(); it != column_value.end(); ++it) {
            if (it->first == key) return it;
        }
        return column_value.end();
    }

    const_iterator find(const string &key) const {
        for (auto it = column_value.begin(); it != column_value.end(); ++it) {
            if (it->first == key) return it;
        }
        return column_value.end();
    }

    vector<pair<string, string>> column_value;
};

class BaseTable {
public: // additional
    using column_val_t = pair<string, string>;
public:
    explicit BaseTable(const string &table_name, const shared_ptr<SqlObj> &sql, bool check_availability = true);
    virtual ~BaseTable() = default;

    virtual void printEntry() const;
    virtual void truncate();

protected:
    virtual bool _getQueryData(const shared_ptr<ResultSet> &sql_res) { return true;}

    virtual string _concat(const column_val_t &cl_val) const;
    virtual string _concatAllRows() const;
    virtual string _concatInsert() const;
    virtual string _concatSet() const;

protected:
    string              _table_name;

    shared_ptr<SqlObj>  _sql;

    RowEntry            _row_entry;
};

class SingleRowTable : public BaseTable {
public:
    explicit SingleRowTable(const string &table_name, const shared_ptr<SqlObj> &sql, bool check_availability = true);
    ~SingleRowTable() = default;

    bool execute();
    bool findByID(const string &id);
//    bool findByColumnValue(const column_val_t &cl_val);

    string  getID() const;
    string  getValueByColumn(const string &column) const;

    void updateColumnValue(column_val_t column_value);
    void setColumnValue(column_val_t column_value);
    void setExecuteType(ExecuteType exec_type) { _exec_type = exec_type; }
    void reset();

private:
    bool _getQueryData(const shared_ptr<ResultSet> &sql_res) override;

    void _fetchRow(shared_ptr<ResultSet> &sql_res, shared_ptr<Statement> &stmt);

private:
    ExecuteType         _exec_type {ExecuteType::ReadyForInsert};
};

class TableCollection : public BaseTable {
public: //additional
    using iterator = vector<SingleRowTable>::iterator;
    using const_iterator = vector<SingleRowTable>::const_iterator;

    iterator begin() { return _collection.begin(); }
    const_iterator begin() const { return _collection.begin(); }

    iterator end() { return _collection.end(); }
    const_iterator end() const { return _collection.end(); }

public: // methods
    explicit TableCollection(const string &table_name, const shared_ptr<SqlObj> &sql);
    ~TableCollection() = default;

    void setColumnValue(column_val_t column_value);
    void findArray();

    size_t getCount() const;

private: // methods
    string _concatAllRows() const final;

    bool _getQueryData(const shared_ptr<ResultSet> &sql_res) override;

private: // members
    map<string, string> _clm_value;

    vector<SingleRowTable> _collection;
};

class DBConnection {
public:
    static DBConnection loadDB(const string &host, const string &user,
                 const string &pass, const string &schema);

    SingleRowTable  getSingleRowTable(const string &table_name);
    TableCollection getTableCollection(const string &table_name);

    void setHost(string &host) {_host = move(host);}
    void setUser(string &user) {_user = move(user);}
    void setPassword(string &password) {_password = move(password);}
    void setSchema(string &schema) {_schema = move(schema);}

    bool isSuccessConn() const {return _is_success_conn;}

private:
    explicit DBConnection(string host, string user, string pass, string schema);

private:
    bool _is_success_conn {false};

    string _host;
    string _user;
    string _password;
    string _schema;

    // Sql data
    shared_ptr<SqlObj> _sql_obj;
};

#endif //SQLWRAPPER_SQLWRAPPER_H
