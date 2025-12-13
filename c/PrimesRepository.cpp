#include "PrimesRepository.h"

#include <sstream>
#include <string>
#include <sql.h>
#include <sqlext.h>

class PrimesRepositoryImpl: public PrimesRepository
{
    public:

        PrimesRepositoryImpl();

        int connect(const char *odbc_connect_string);

        int disconnect();

        int savePrimeMultiplesMap(
                const std::map<uint64_t, uint64_t> &prime_multiples_map);

        int readSavedPrimeMultiples(
                std::map<uint64_t, uint64_t> &prime_multiples_map);

        const char *getLastError();

        void buildLastError(
                SQLHANDLE handle, SQLSMALLINT handle_type,
                const char *error_prefix);

    protected:

        /** ODBC environment handle. */
        SQLHENV _sql_env;

        /** ODBC database connection handle. */
        SQLHDBC _sqldb_connection;

        /** ODBC statement handle, used for directly-specified statements. */
        SQLHSTMT _sql_statement;

        /** ODBC statement handle, prepared to insert records of prime numbers
         * and their largest tested multiple. */
        SQLHSTMT _sql_insert_prime_statement;

        /** ODBC statement handle, prepared to update records of prime numbers
         * and their largest tested multiple. */
        SQLHSTMT _sql_update_prime_statement;

        /** Error text set by methods in this class, retrieved via
         * ::getLastError. */
        std::string _last_error;
};

PrimesRepositoryImpl::PrimesRepositoryImpl()
{
}

int PrimesRepositoryImpl::connect(const char *odbc_connect_string)
{
    bool success = false;

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
                        (SQLCHAR*)odbc_connect_string,
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

        if (!SQL_SUCCEEDED(SQLAllocHandle(
                        SQL_HANDLE_STMT, _sqldb_connection,
                        &_sql_update_prime_statement)))
        {
            buildLastError(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLAllocHandle(update primes statement) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLPrepare(
                        _sql_update_prime_statement,
                        (SQLCHAR*)"UPDATE Primes SET last_multiple = ?"
                        "WHERE value = ?;",
                        SQL_NTS)))
        {
            buildLastError(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLPrepare of update statement failed: ");
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
    SQLFreeHandle(SQL_HANDLE_ENV, _sql_update_prime_statement);
    return 0;
}

int PrimesRepositoryImpl::savePrimeMultiplesMap(
        const std::map<uint64_t, uint64_t> &prime_multiples_map)
{
    uint64_t prime_value, prime_multiple;
    std::map<uint64_t, uint64_t> 
        last_prime_multiples_map = _last_prime_multiples_map;

    _last_error.clear();

    /* Bind parameters for insert statemetnt. */

    if (!SQL_SUCCEEDED(SQLBindParameter(
            _sql_insert_prime_statement,
            1,
            SQL_PARAM_INPUT,
            SQL_C_UBIGINT,
            SQL_BIGINT,
            0, 0,
            &prime_value,
            0, 0)))
    {
        buildLastError(
                _sql_statement, SQL_HANDLE_STMT,
                "SQLBindParameter(prime value) failed: ");
        return -1;
    }

    if (!SQL_SUCCEEDED(SQLBindParameter(
            _sql_insert_prime_statement,
            2,
            SQL_PARAM_INPUT,
            SQL_C_UBIGINT,
            SQL_BIGINT,
            0, 0,
            &prime_multiple,
            0, 0)))
    {
            buildLastError(
                    _sql_statement, SQL_HANDLE_STMT,
                    "SQLBindParameter(prime multiple) failed: ");
            return -1;
    }

    /* Bind parameters for update statemetnt. */

    if (!SQL_SUCCEEDED(SQLBindParameter(
            _sql_update_prime_statement,
            1,
            SQL_PARAM_INPUT,
            SQL_C_UBIGINT,
            SQL_BIGINT,
            0, 0,
            &prime_value,
            0, 0)))
    {
        buildLastError(
                _sql_statement, SQL_HANDLE_STMT,
                "SQLBindParameter(prime value) failed: ");
        return -1;
    }

    if (!SQL_SUCCEEDED(SQLBindParameter(
            _sql_update_prime_statement,
            2,
            SQL_PARAM_INPUT,
            SQL_C_UBIGINT,
            SQL_BIGINT,
            0, 0,
            &prime_multiple,
            0, 0)))
    {
            buildLastError(
                    _sql_statement, SQL_HANDLE_STMT,
                    "SQLBindParameter(prime multiple) failed: ");
            return -1;
    }

    for (auto map_iter: prime_multiples_map)
    {
        auto last_prime_multiple = last_prime_multiples_map.find(
                map_iter.first);

        if (last_prime_multiple == last_prime_multiples_map.end())
        {
            prime_value = map_iter.first;
            prime_multiple = map_iter.second;
            if (!SQL_SUCCEEDED(
                        SQLExecute(_sql_insert_prime_statement)))
            {
                buildLastError(
                        _sql_insert_prime_statement, SQL_HANDLE_STMT,
                        "SQLExecute(insert prime multiple) failed: ");
                return -1;
            }
        }
        else if (last_prime_multiple->second != map_iter.second)
        {
            prime_value = map_iter.first;
            prime_multiple = map_iter.second;
            if (SQLExecute(_sql_update_prime_statement) != SQL_NO_DATA)
            {
                buildLastError(
                        _sql_update_prime_statement, SQL_HANDLE_STMT,
                        "SQLExecute(update prime multiple) failed: ");
                return -1;
            }
            continue;
        }
    }

    _last_prime_multiples_map = prime_multiples_map;

    return 0;
}


int PrimesRepositoryImpl::readSavedPrimeMultiples(
        std::map<uint64_t, uint64_t> &prime_multiples_map)
{
    SQLHSTMT select_statement;
    SQLRETURN rc;
    std::map<uint64_t, uint64_t> new_prime_multiples_map;
    uint64_t prime_value, prime_multiple;
    bool success = false;

    if (!SQL_SUCCEEDED(SQLAllocHandle(
                    SQL_HANDLE_STMT, _sqldb_connection,
                    &select_statement)))
    {
        buildLastError(
                _sqldb_connection, SQL_HANDLE_DBC,
                "SQLAllocHandle(select statement) failed: ");
        return -1;
    }

    do
    {
        if (!SQL_SUCCEEDED(SQLPrepare(
                        select_statement,
                        (SQLCHAR*)"SELECT value, last_multiple"
                        " FROM Primes;", SQL_NTS)))
        {
            buildLastError(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLPrepare of select statement failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLBindCol(
                        select_statement,
                        1,
                        SQL_C_UBIGINT,
                        &prime_value,
                        0, 0)))
        {
            buildLastError(
                    select_statement, SQL_HANDLE_STMT,
                    "SQLBindCol(prime value) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLBindCol(
                        select_statement,
                        2,
                        SQL_C_UBIGINT,
                        &prime_multiple,
                        0, 0)))
        {
            buildLastError(
                    select_statement, SQL_HANDLE_STMT,
                    "SQLBindCol(last prime multiple) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLExecute(select_statement)))
        {
            buildLastError(
                    select_statement, SQL_HANDLE_STMT,
                    "SQLExecute(select prime values) failed: ");
            break;
        }

        do
        {
            rc = SQLFetch(select_statement);
            if (rc != SQL_SUCCESS)
                break;

            new_prime_multiples_map.insert(
                    std::make_pair(prime_value, prime_multiple));

        } while (true);

        if (rc != SQL_NO_DATA_FOUND)
        {
            buildLastError(
                    select_statement, SQL_HANDLE_STMT,
                    "SQLFetch(select prime values) failed: ");
            break;
        }

        success = true;

    } while(0);

    SQLFreeHandle(SQL_HANDLE_STMT, select_statement);

    if (!success)
        return -1;

    prime_multiples_map = new_prime_multiples_map;

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
