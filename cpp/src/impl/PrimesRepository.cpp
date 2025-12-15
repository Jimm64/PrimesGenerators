#include "PrimesRepository.h"

#include <sstream>
#include <string>
#include <sql.h>
#include <sqlext.h>

PrimesRepository::~PrimesRepository()
{
}

class PrimesRepositoryImpl: public PrimesRepository
{
    public:

        PrimesRepositoryImpl();

        ~PrimesRepositoryImpl();

        int connect(const char *odbc_connect_string);

        int disconnect();

        int savePrimeMultiplesMap(
                const std::map<uint64_t, uint64_t> &prime_multiples_map);

        int commit();

        int readSavedPrimeMultiples();

        const char *getLastError();

        const std::map<uint64_t, uint64_t> getPrimeMultiplesMap();

    protected:

        /**
         * @brief Construct eror text for ::getLastError based on the given
         * ODBC handle.
         *
         * @param handle ODBC handle
         * @param handle_type Type of the ODBC handle
         * @param error_prefix An initial string of text to provide.
         */
        void buildLastErrorForSqlDiagnostics(
                SQLHANDLE handle, SQLSMALLINT handle_type,
                const char *error_prefix);

        /**
         * @brief Cleanup handles representing the ODBC environment.
         */
        void cleanupOdbcEnvironment();

        /** ODBC environment handle. */
        SQLHENV _sql_env;

        /** ODBC database connection handle. */
        SQLHDBC _sqldb_connection;

        /** ODBC statement handle, prepared to insert records of prime numbers
         * and their largest tested multiple. */
        SQLHSTMT _sql_insert_prime_statement;

        /** ODBC statement handle, prepared to update records of prime numbers
         * and their largest tested multiple. */
        SQLHSTMT _sql_update_prime_statement;

        /** Error text set by methods in this class, retrieved via
         * ::getLastError. */
        std::string _last_error;

        /** Mapping of known primes to the largest multiple that has been used
         * by e.g. ::PrimesGenerator to find new prime numbers. */
        std::map<uint64_t, uint64_t> _last_prime_multiples_map;
};

PrimesRepositoryImpl::PrimesRepositoryImpl()
{
    _sql_env = NULL;
    _sqldb_connection = NULL;
    _sql_insert_prime_statement = NULL;
    _sql_update_prime_statement = NULL;
}

PrimesRepositoryImpl::~PrimesRepositoryImpl()
{
    cleanupOdbcEnvironment();
}

int PrimesRepositoryImpl::connect(const char *odbc_connect_string)
{
    SQLHSTMT create_table_statement;
    bool success = false;

    _last_error.clear();

    if (_sqldb_connection != NULL)
    {
        _last_error = "Already connected.";
        return -1;
    }

    do
    {
        /* Create database environment and connection handles. */

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
            buildLastErrorForSqlDiagnostics(
                    _sql_env, SQL_HANDLE_ENV,
                    "SQLAllocHandle(connection) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLSetConnectAttr(
                        _sqldb_connection, SQL_ATTR_AUTOCOMMIT,
                        (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER)))
        {
            buildLastErrorForSqlDiagnostics(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLSetConnectAttr(autocommit off) failed: ");
            break;
        }

        /* Connect to the database. */


        if (!SQL_SUCCEEDED(SQLDriverConnect(
                        _sqldb_connection, NULL,
                        (SQLCHAR*)odbc_connect_string,
                        SQL_NTS,
                        NULL, 0, NULL, SQL_DRIVER_COMPLETE)))
        {
            buildLastErrorForSqlDiagnostics(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLDriverConnect failed: ");
            break;
        }

        /* Allocate and prepare reusable database statement handles for
         * inserting and updating prime-multiple mapping entries. */

        if (!SQL_SUCCEEDED(SQLAllocHandle(
                        SQL_HANDLE_STMT, _sqldb_connection,
                        &_sql_insert_prime_statement)))
        {
            buildLastErrorForSqlDiagnostics(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLAllocHandle(insert primes statement) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLPrepare(
                        _sql_insert_prime_statement,
                        (SQLCHAR*)"INSERT INTO Primes VALUES(?, ?)", SQL_NTS)))
        {
            buildLastErrorForSqlDiagnostics(
                    _sql_insert_prime_statement, SQL_HANDLE_STMT,
                    "SQLPrepare of insert statement failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLAllocHandle(
                        SQL_HANDLE_STMT, _sqldb_connection,
                        &_sql_update_prime_statement)))
        {
            buildLastErrorForSqlDiagnostics(
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
            buildLastErrorForSqlDiagnostics(
                    _sql_update_prime_statement, SQL_HANDLE_STMT,
                    "SQLPrepare of update statement failed: ");
            break;
        }

        /* Create the primes table if it doesn't already exist. */

        if (!SQL_SUCCEEDED(SQLAllocHandle(
                        SQL_HANDLE_STMT, _sqldb_connection,
                        &create_table_statement)))
        {
            buildLastErrorForSqlDiagnostics(
                    _sqldb_connection, SQL_HANDLE_DBC,
                    "SQLAllocHandle(statement) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLExecDirect(
                        create_table_statement,
                        (SQLCHAR*)"CREATE TABLE IF NOT EXISTS Primes("
                        "value INTEGER,"
                        "last_multiple INTEGER);",
                        SQL_NTS)))
        {
            buildLastErrorForSqlDiagnostics(
                    create_table_statement, SQL_HANDLE_STMT,
                    "Primes table creation failed: ");
            SQLFreeHandle(SQL_HANDLE_STMT, create_table_statement);
            break;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, create_table_statement);

        success = true;

    } while (false);

    if (!success)
    {
        cleanupOdbcEnvironment();
        return -1;
    }

    return 0;
}

void PrimesRepositoryImpl::cleanupOdbcEnvironment()
{
    if (_sql_update_prime_statement != NULL)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, _sql_update_prime_statement);
        _sql_update_prime_statement = NULL;
    }

    if (_sql_insert_prime_statement)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, _sql_insert_prime_statement);
        _sql_insert_prime_statement = NULL;
    }

    if (_sqldb_connection != NULL)
    {
        SQLFreeHandle(SQL_HANDLE_DBC, _sqldb_connection);
        _sqldb_connection = NULL;
    }

    if (_sql_env != NULL)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, _sql_env);
        _sql_env = NULL;
    }
}

int PrimesRepositoryImpl::disconnect()
{
    _last_error.clear();

    if (_sqldb_connection == NULL)
    {
        _last_error = "Not connected to a database.";
        return -1;
    }

    if (!SQL_SUCCEEDED(SQLDisconnect(_sqldb_connection)))
    {
        buildLastErrorForSqlDiagnostics(
                _sqldb_connection, SQL_HANDLE_DBC,
                "SQLDisconnect() failed: ");
        return -1;
    }

    cleanupOdbcEnvironment();
    return 0;
}

int PrimesRepositoryImpl::savePrimeMultiplesMap(
        const std::map<uint64_t, uint64_t> &prime_multiples_map)
{
    uint64_t prime_value, prime_multiple;
    std::map<uint64_t, uint64_t> 
        last_prime_multiples_map = _last_prime_multiples_map;

    _last_error.clear();

    if (_sqldb_connection == NULL)
    {
        _last_error = "Not connected to a database.";
        return -1;
    }

    /* Bind parameters for insert statement (prime value, multiple). */

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
        buildLastErrorForSqlDiagnostics(
                _sql_insert_prime_statement, SQL_HANDLE_STMT,
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
            buildLastErrorForSqlDiagnostics(
                    _sql_insert_prime_statement, SQL_HANDLE_STMT,
                    "SQLBindParameter(prime multiple) failed: ");
            return -1;
    }

    /* Bind parameters for update statement (multiple, prime value). */

    if (!SQL_SUCCEEDED(SQLBindParameter(
            _sql_update_prime_statement,
            1,
            SQL_PARAM_INPUT,
            SQL_C_UBIGINT,
            SQL_BIGINT,
            0, 0,
            &prime_multiple,
            0, 0)))
    {
            buildLastErrorForSqlDiagnostics(
                    _sql_update_prime_statement, SQL_HANDLE_STMT,
                    "SQLBindParameter(prime multiple) failed: ");
            return -1;
    }

    if (!SQL_SUCCEEDED(SQLBindParameter(
            _sql_update_prime_statement,
            2,
            SQL_PARAM_INPUT,
            SQL_C_UBIGINT,
            SQL_BIGINT,
            0, 0,
            &prime_value,
            0, 0)))
    {
        buildLastErrorForSqlDiagnostics(
                _sql_update_prime_statement, SQL_HANDLE_STMT,
                "SQLBindParameter(prime value) failed: ");
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
                buildLastErrorForSqlDiagnostics(
                        _sql_insert_prime_statement, SQL_HANDLE_STMT,
                        "SQLExecute(insert prime multiple) failed: ");
                return -1;
            }
        }
        else if (last_prime_multiple->second != map_iter.second)
        {
            prime_value = map_iter.first;
            prime_multiple = map_iter.second;
            if (!SQL_SUCCEEDED(SQLExecute(_sql_update_prime_statement)))
            {
                buildLastErrorForSqlDiagnostics(
                        _sql_update_prime_statement, SQL_HANDLE_STMT,
                        "SQLExecute(update prime multiple) failed: ");
                return -1;
            }
        }
    }


    _last_prime_multiples_map = prime_multiples_map;

    return 0;
}

int PrimesRepositoryImpl::commit()
{
    _last_error.clear();

    if (_sqldb_connection == NULL)
    {
        _last_error = "Not connected to a database.";
        return -1;
    }

    if (!SQL_SUCCEEDED(SQLEndTran(
                    SQL_HANDLE_DBC, _sqldb_connection, SQL_COMMIT)))
    {
        buildLastErrorForSqlDiagnostics(
                _sqldb_connection, SQL_HANDLE_DBC,
                "SQLEndTran() failed: ");
        return -1;
    }

    return 0;
}


int PrimesRepositoryImpl::readSavedPrimeMultiples()
{
    SQLHSTMT select_statement;
    SQLRETURN rc;
    std::map<uint64_t, uint64_t> new_prime_multiples_map;
    uint64_t prime_value, prime_multiple;
    bool success = false;

    _last_error.clear();

    if (!SQL_SUCCEEDED(SQLAllocHandle(
                    SQL_HANDLE_STMT, _sqldb_connection,
                    &select_statement)))
    {
        buildLastErrorForSqlDiagnostics(
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
            buildLastErrorForSqlDiagnostics(
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
            buildLastErrorForSqlDiagnostics(
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
            buildLastErrorForSqlDiagnostics(
                    select_statement, SQL_HANDLE_STMT,
                    "SQLBindCol(last prime multiple) failed: ");
            break;
        }

        if (!SQL_SUCCEEDED(SQLExecute(select_statement)))
        {
            buildLastErrorForSqlDiagnostics(
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
            buildLastErrorForSqlDiagnostics(
                    select_statement, SQL_HANDLE_STMT,
                    "SQLFetch(select prime values) failed: ");
            break;
        }

        success = true;

    } while(0);

    SQLFreeHandle(SQL_HANDLE_STMT, select_statement);

    if (!success)
        return -1;

    _last_prime_multiples_map = new_prime_multiples_map;

    return 0;
}

const std::map<uint64_t, uint64_t> PrimesRepositoryImpl::getPrimeMultiplesMap()
{
    return _last_prime_multiples_map;
}

const char *PrimesRepositoryImpl::getLastError()
{
    return _last_error.c_str();
}

PrimesRepository *PrimesRepository::create()
{
    return new PrimesRepositoryImpl;
}

void PrimesRepositoryImpl::buildLastErrorForSqlDiagnostics(
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
