#include "database.h"

database::database()
{
    connectionString = (SQLCHAR*)"DRIVER={SQL Server Native Client 11.0};SERVER=192.168.1.20;DATABASE=producao;UID=mir;PWD=-mda8960;";
}

database::~database()
{
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);
}

vector<UrlStream* >  database::getRadiosActive(string guid)
{
    if(isGuid(guid))
        throw;

    SQLCHAR retconstring[1024];

    //string query = "select oid, Name from radio";

    string query = "select Radio, urlstreaming from UrlStream u left join Radio r on r.OID = u.Radio where r.Listener = cast('239B5AFB-8700-4080-85C1-95ED594FA876' as uniqueidentifier) and r.IsActive = 1";
    vector<UrlStream*> urlstream ;

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvhandle))
        throw;

    if(SQL_SUCCESS!=SQLSetEnvAttr(sqlEnvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        throw;

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvhandle, &sqlConnectionhandle))
        throw;

    //Abre conecção com SQL.
    RETCODE recode = SQLDriverConnect(sqlConnectionhandle, NULL, connectionString, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

    switch(recode)
    {
    case SQL_SUCCESS_WITH_INFO:
        showError(SQL_HANDLE_DBC, sqlConnectionhandle);
        break;
    case SQL_INVALID_HANDLE:
        throw;
    case SQL_ERROR:
        showError(SQL_HANDLE_DBC, sqlConnectionhandle);
        throw;
    default:
        break;
    }

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        showError(SQL_HANDLE_DBC, sqlConnectionhandle);
        throw;
    }



    if(SQL_SUCCESS!=SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        showError(SQL_HANDLE_DBC, sqlConnectionhandle);
        throw;
    }
    else
    {
        int radio;
        char address[128];

        while(SQLFetch(sqlStatementhandle)==SQL_SUCCESS)
        {
            SQLGetData(sqlStatementhandle, 1, SQL_C_CHAR, &radio, 0, NULL);
            SQLGetData(sqlStatementhandle, 2, SQL_C_CHAR, address, 128, NULL);

            UrlStream* us = new UrlStream();
            us->radio = radio;
            us->urlStream = string(address);

            urlstream.push_back(us);
        }
    }
    return urlstream;
}

void database::saveLog(int radioId,string codeLog)
{
    SQLCHAR retconstring[1024];

    string query = "exec Radio_log " + to_string(radioId) + ", " + codeLog + ", " + getDateSqlString();

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvhandle))
        throw;

    if(SQL_SUCCESS!=SQLSetEnvAttr(sqlEnvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        throw;

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvhandle, &sqlConnectionhandle))
        throw;

    //Abre conecção com SQL.
    RETCODE recode = SQLDriverConnect(sqlConnectionhandle, NULL, connectionString, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

    switch(recode)
    {
    case SQL_SUCCESS_WITH_INFO:
        showError(SQL_HANDLE_DBC, sqlConnectionhandle);
        break;
    case SQL_INVALID_HANDLE:
        throw;
    case SQL_ERROR:
        showError(SQL_HANDLE_DBC, sqlConnectionhandle);
        throw;
    default:
        break;
    }

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        showError(SQL_HANDLE_DBC, sqlConnectionhandle);
        throw;
    }

    if(SQL_SUCCESS!=SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        showError(SQL_HANDLE_DBC, sqlConnectionhandle);
        throw;
    }
}

string database::getDateSqlString()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [18];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,80,"%F %T",timeinfo);

    return string(buffer);
}

bool database::isGuid(string guid)
{
    if(!guid.empty() && guid.size() == 36 && guid[9] == '-' && guid[14] == '-' && guid[19] == '-' && guid[24]== '-')
        return true;
}

void database::showError(unsigned int handletype, const SQLHANDLE& handle)
{
    SQLCHAR sqlstate[1024];
    SQLCHAR message[1024];
    if(SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL))
        cout<<"\nMessage: "<<message<<"\nSQLSTATE: "<<sqlstate<<endl;
}
