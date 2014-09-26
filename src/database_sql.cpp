#include "database_sql.h"


Database_SQL::Database_SQL(std::string sqlConnect)
{
    //connectionString = (SQLCHAR*)"DRIVER={SQL Server Native Client 11.0};SERVER=192.168.1.20;DATABASE=producao;UID=mir;PWD=-mda8960;";
    connectionString = (SQLCHAR*)sqlConnect.c_str();
}

Database_SQL::~Database_SQL()
{

}

vector<UrlStream* >  Database_SQL::getRadiosActive(string guid)
{
    SQLCHAR retconstring[1024];

    //construo a query que fará a chamad
    string query ="select (select top 1 oid from UrlStream u where u.Radio = ra.Oid AND u.Active = 1 order by u.Ordinal) Oid,ra.OID radio,(select top 1 UrlStreaming from UrlStream u where u.Radio = ra.Oid AND u.Active = 1 order by u.Ordinal) urlstreaming from Radio ra where ra.Listener = '" + guid + "' and ra.IsActive = 1 and (select top 1 UrlStreaming from UrlStream u where u.Radio = ra.Oid AND u.Active = 1 order by u.Ordinal) is not null order by ra.StarRating";
    vector<UrlStream*> urlstream ;

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_ENV);

    if(SQL_SUCCESS!=SQLSetEnvAttr(sqlEnvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        throw SqlServerException() << errno_code(MIR_DB_SET_ENV);

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvhandle, &sqlConnectionhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_HANDLE);

    //Abre conecção com SQL.
    RETCODE recode = SQLDriverConnect(sqlConnectionhandle, NULL, connectionString, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

    switch(recode)
    {
    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
        throw SqlServerException() << errno_code(MIR_DB_DRIVER_CONNECT);
        logError(SQL_HANDLE_DBC, sqlConnectionhandle);
    default:
        break;
    }

    if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_STMT);
    }

    //SQLExecute()
    if(SQL_SUCCESS != SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        throw SqlServerException() << errno_code(MIR_DB_EXECUTE_QUERY);
    }
    else
    {
        int oid;
        int radio;
        char address[128];

        while(SQLFetch(sqlStatementhandle)==SQL_SUCCESS)
        {
            SQLGetData(sqlStatementhandle, 1, SQL_C_LONG, &oid, 0, NULL);
            SQLGetData(sqlStatementhandle, 2, SQL_C_LONG, &radio, 0, NULL);
            SQLGetData(sqlStatementhandle, 3, SQL_C_CHAR, address, 128, NULL);

            UrlStream* us = new UrlStream();
            us->id = oid;
            us->radio = radio;
            us->url = string(address);

            urlstream.push_back(us);
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLDisconnect(sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);

    return urlstream;
}

void Database_SQL::saveLog(int radioId, CodeLog codeLog)
{
    SQLCHAR retconstring[1024];
    string code= "";

    switch (codeLog)
    {
    case CAPTURE_SUCESS:
        code = "MOT007";
        break;
    case CAPTURE_ERROR:
        code = "MOT005";
        break;
    case CAPTURE_BEGIN:
        code = "MOT001";
        break;
    case CAPTURE_END:
        code = "MOT002";
        break;
    }

    string query = "exec sp_RadioLog " + to_string(radioId) + ", '" + code + "', '" + getDateSqlString() + "'";

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_ENV);

    if(SQL_SUCCESS!=SQLSetEnvAttr(sqlEnvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        throw SqlServerException() << errno_code(MIR_DB_SET_ENV);

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvhandle, &sqlConnectionhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_HANDLE);

    //Abre conecção com SQL.
    RETCODE recode = SQLDriverConnect(sqlConnectionhandle, NULL, connectionString, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

    switch(recode)
    {

    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
        throw SqlServerException() << errno_code(MIR_DB_DRIVER_CONNECT);
        logError(SQL_HANDLE_DBC, sqlConnectionhandle);
    default:
        break;
    }

    if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_STMT);
    }

    //SQLExecute()
    if(SQL_SUCCESS != SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        throw SqlServerException() << errno_code(MIR_DB_EXECUTE_QUERY);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLDisconnect(sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);
}

int Database_SQL::insertCutHistory(int radio, string path, string dateTime )
{
    SQLCHAR retconstring[1024];
    int retornoCutHistoryId = 0;

    string query = "SET NOCOUNT ON INSERT INTO CutHistory (radi_cd_radio, cuhi_tx_caminho_do_arquivo, cuhi_dt_hora_da_captura) OUTPUT inserted.cuhi_cd_cuthistory VALUES ( "+ to_string(radio) + ", '" +path+ "', '" +dateTime+ "')";

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_ENV);

    if(SQL_SUCCESS!=SQLSetEnvAttr(sqlEnvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        throw SqlServerException() << errno_code(MIR_DB_SET_ENV);

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvhandle, &sqlConnectionhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_HANDLE);

    //Abre conecção com SQL.
    RETCODE recode = SQLDriverConnect(sqlConnectionhandle, NULL, connectionString, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

    switch(recode)
    {

    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
        throw SqlServerException() << errno_code(MIR_DB_DRIVER_CONNECT);
        logError(SQL_HANDLE_DBC, sqlConnectionhandle);
    default:
        break;
    }

    if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_STMT);
    }

    //SQLExecute()
    if(SQL_SUCCESS != SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        throw SqlServerException() << errno_code(MIR_DB_EXECUTE_QUERY);
    }
    else
    {
        if(SQLFetch(sqlStatementhandle) == SQL_SUCCESS)
            SQLGetData(sqlStatementhandle, 1, SQL_C_LONG, &retornoCutHistoryId, 0, NULL);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLDisconnect(sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);

    return retornoCutHistoryId;
}

void Database_SQL::updateCutHistory(int cutHistoryId, int songRecognized, string dateTime)
{
    Database_SQL::updateCutHistory(cutHistoryId, songRecognized, dateTime, false);
}

void Database_SQL::updateCutHistory(int cutHistoryId, int songRecognized, string dateTime, bool silenceDetect)
{
    if  (cutHistoryId == 0)
        return;

    SQLCHAR retconstring[1024];

    int silenceConvertToIntSql =  silenceDetect ? 1 : 0;

    string query = "UPDATE CutHistory SET  cuhi_dt_hora_da_processamento = '"+ dateTime +"', song_cd_oid = "+to_string(songRecognized)+" , cuhi_in_silencio = "+ to_string(silenceDetect) +" WHERE cuhi_cd_cuthistory = " + to_string(cutHistoryId);

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_ENV);

    if(SQL_SUCCESS!=SQLSetEnvAttr(sqlEnvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        throw SqlServerException() << errno_code(MIR_DB_SET_ENV);

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvhandle, &sqlConnectionhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_HANDLE);

    //Abre conecção com SQL.
    RETCODE recode = SQLDriverConnect(sqlConnectionhandle, NULL, connectionString, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

    switch(recode)
    {
    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
        throw SqlServerException() << errno_code(MIR_DB_DRIVER_CONNECT);
        logError(SQL_HANDLE_DBC, sqlConnectionhandle);
    default:
        break;
    }

    if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_STMT);
    }

    //SQLExecute()
    if(SQL_SUCCESS != SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        throw SqlServerException() << errno_code(MIR_DB_EXECUTE_QUERY);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLDisconnect(sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);
}

string Database_SQL::getDateSqlString()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [18];

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,80,"%F %T",timeinfo);
    return string(buffer);
}

void Database_SQL::insertRecognitionOccurrence(int radio, int song, string datetime, string pathMp3)
{
    SQLCHAR retconstring[1024];

    string query = "exec sp_RecognitionOccurrence_insert @RADIO=" + to_string(radio) + ",@OccurrenceDate='" + datetime  + "',@Filename='" + pathMp3+ "',@Song=" + to_string(song);

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_ENV);

    if(SQL_SUCCESS!=SQLSetEnvAttr(sqlEnvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        throw SqlServerException() << errno_code(MIR_DB_SET_ENV);

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvhandle, &sqlConnectionhandle))
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_HANDLE);

    //Abre conecção com SQL.
    RETCODE recode = SQLDriverConnect(sqlConnectionhandle, NULL, connectionString, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

    switch(recode)
    {

    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
        throw SqlServerException() << errno_code(MIR_DB_DRIVER_CONNECT);
        logError(SQL_HANDLE_DBC, sqlConnectionhandle);
    default:
        break;
    }

    if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        throw SqlServerException() << errno_code(MIR_DB_ALLOC_STMT);
    }

    //SQLExecute()
    if(SQL_SUCCESS != SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        throw SqlServerException() << errno_code(MIR_DB_EXECUTE_QUERY);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLDisconnect(sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);}

void Database_SQL::logError(unsigned int handletype, const SQLHANDLE& handle)
{
    SQLCHAR sqlstate[1024];
    SQLCHAR message[1024];

    if(SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL))
        printf("\nMessage: %s \t SQLSTATE: %s\n",message,sqlstate);

    return ;
}





