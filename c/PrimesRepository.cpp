#include "PrimesRepository.h"

#include <sstream>
#include <string>
#include <sql.h>
#include <sqlext.h>

class PrimesRepositoryImpl: public PrimesRepository
{
    public:

        PrimesRepositoryImpl();

        int connect();

        int disconnect();

        int savePrime(uint64_t value);

        const char *getLastError();

        void buildLastError(
                SQLHANDLE handle, SQLSMALLINT handle_type,
                const char *error_prefix);

    protected:

        SQLHENV _sql_env;
        SQLHDBC _sqldb_connection;
        SQLHSTMT _sql_statement;
        SQLHSTMT _sql_insert_prime_statement;
        SQLHDESC _sql_desc;
        std::string _last_error;
};

PrimesRepositoryImpl::PrimesRepositoryImpl()
{
}

int PrimesRepositoryImpl::connect()
{
    bool success = false;
    SQLCHAR *connect_string = (SQLCHAR*)
        "DRIVER=SQLITE3;Database=./primes.sqlite3;";

    _last_error.clear();

    do
    {
        /* Create database environment and connection. */

        if (!SQL_SUCCEEDED(SQLAllocHandle(
                        SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_sql_env)))
        {
            _last_error = "SQLAllocHandle(envirnoment) failed.";
            break;
        }

        SQLSetEnvAttr(
                _sql_env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

        if (!SQL_SUCCEEDED(SQLAllocHandle(
                        SQL_HANDLE_DBC, _sql_env, &_sqldb_connection)))
        {
            buildLastError(
                    _sql_env, SQL_HANDLE_ENV,
                    "SQLAllocHandle(connection) failed: ");
            break;
        }


        if (!SQL_SUCCEEDED(SQLDriverConnect(
                        _sqldb_connection, NULL,
                        connect_string,
                        SQL_NTS,
                        NULL, 0, NULL, SQL_DRIVER_COMPLETE)))
        {
            buildLastError(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLDriverConnect failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLAllocHandle(
                        SQL_HANDLE_STMT, _sqldb_connection,
                        &_sql_statement)))
        {
            buildLastError(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLAllocHandle(statement) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLAllocHandle(
                        SQL_HANDLE_STMT, _sqldb_connection,
                        &_sql_insert_prime_statement)))
        {
            buildLastError(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLAllocHandle(insert primes statement) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLPrepare(
                        _sql_insert_prime_statement,
                        (SQLCHAR*)"INSERT INTO Primes VALUES(?, ?)", SQL_NTS)))
        {
            buildLastError(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLPrepare of insert statement failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLExecDirect(
                        _sql_statement,
                        (SQLCHAR*)"CREATE TABLE IF NOT EXISTS Primes("
                        "value INTEGER,"
                        "last_multiple INTEGER);",
                        SQL_NTS)))
        {
            buildLastError(
                    _sql_statement, SQL_HANDLE_STMT,
                    "Primes table creation failed: ");
            break;
        }


        success = true;

    } while (false);

    if (!success)
        return -1;

    return 0;
}

int PrimesRepositoryImpl::disconnect()
{
    _last_error.clear();
    SQLFreeHandle(SQL_HANDLE_ENV, _sql_env);
    SQLFreeHandle(SQL_HANDLE_ENV, _sqldb_connection);
    SQLFreeHandle(SQL_HANDLE_ENV, _sql_statement);
    SQLFreeHandle(SQL_HANDLE_ENV, _sql_insert_prime_statement);
    return 0;
}

int PrimesRepositoryImpl::savePrime(uint64_t value)
{
    if (!SQL_SUCCEEDED(SQLBindParameter(
            _sql_insert_prime_statement,
            1,
            SQL_PARAM_INPUT,
            SQL_C_UBIGINT,
            SQL_BIGINT,
            0, 0,
            &value,
            0, 0)))
    {
            buildLastError(
                    _sql_statement, SQL_HANDLE_STMT,
                    "SQLBindParameter(value) failed: ");
            return -1;
    }

    if (!SQL_SUCCEEDED(SQLBindParameter(
            _sql_insert_prime_statement,
            2,
            SQL_PARAM_INPUT,
            SQL_C_UBIGINT,
            SQL_BIGINT,
            0, 0,
            &value,
            0, 0)))
    {
            buildLastError(
                    _sql_statement, SQL_HANDLE_STMT,
                    "SQLBindParameter(last_multiple) failed: ");
            return -1;
    }

    if (!SQL_SUCCEEDED(
                SQLExecute(_sql_insert_prime_statement)))
    {
            buildLastError(
                    _sql_statement, SQL_HANDLE_STMT,
                    "SQLExecute(insert prime value) failed: ");
        return -1;
    }

    _last_error.clear();

    return 0;
}

const char *PrimesRepositoryImpl::getLastError()
{
    return _last_error.c_str();
}

PrimesRepository *PrimesRepository::create()
{
    return new PrimesRepositoryImpl;
}

void PrimesRepositoryImpl::buildLastError(
        SQLHANDLE handle, SQLSMALLINT handle_type,
        const char *error_prefix)
{
    SQLINTEGER i = 0;
    SQLINTEGER native;
    SQLCHAR state[7];
    SQLCHAR text[256];
    SQLSMALLINT len;
    SQLRETURN rc;
    std::stringstream error_stream;

    if (error_prefix != NULL)
        error_stream << error_prefix;

    do
    {
        rc = SQLGetDiagRec(
                handle_type, handle,
                ++i,
                state,
                &native,
                text, sizeof(text),
                &len);

        if (!SQL_SUCCEEDED(rc))
            break;

        error_stream << i << ": " << text << ";";

    } while (true);

    _last_error = error_stream.str();

}
