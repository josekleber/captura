#include "database.h"

database::database()
{
    connectionString = (SQLCHAR*)"DRIVER={SQL Server Native Client 11.0};SERVER=192.168.1.20;DATABASE=producao;UID=mir;PWD=-mda8960;";
}

database::~database()
{

}

vector<UrlStream* >  database::getRadiosActive(string guid)
{
    SQLCHAR retconstring[1024];

    //construo a query que fará a chamad
    string query = "SELECT u.oid, u.Radio, u.urlstreaming FROM UrlStream u LEFT JOIN Radio r ON r.OID = u.Radio WHERE r.Listener = '" + guid + "' AND r.IsActive = 1";
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
    //SQLExecute()
    {
    case SQL_INVALID_HANDLE:
        throw;
    case SQL_ERROR:
        throw;
    default:
        break;
    }

    if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        throw;
    }

    //SQLExecute()
    if(SQL_SUCCESS != SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        throw;
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
            us->urlStream = string(address);

            urlstream.push_back(us);
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);

    return urlstream;
}

void database::saveLog(int radioId, CodeLog codeLog)
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
        throw;

    if(SQL_SUCCESS!=SQLSetEnvAttr(sqlEnvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        throw;

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvhandle, &sqlConnectionhandle))
        throw;

    //Abre conecção com SQL.
    RETCODE recode = SQLDriverConnect(sqlConnectionhandle, NULL, connectionString, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

    switch(recode)
    {
    case SQL_INVALID_HANDLE:
        throw;
    case SQL_ERROR:
        throw;
    default:
        break;
    }

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        throw;
    }

    if(SQL_SUCCESS!=SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        throw;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);

}

int database::insertCutHistory(int radio, string path, string dateTime )
{
    SQLCHAR retconstring[1024];
    int retornoCutHistoryId = 0;

    string query = "SET NOCOUNT ON INSERT INTO CutHistory (radi_cd_radio, cuhi_tx_caminho_do_arquivo, cuhi_dt_hora_da_captura) OUTPUT inserted.cuhi_cd_cuthistory VALUES ( "+ to_string(radio) + ", '" +path+ "', '" +dateTime+ "')";

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
    case SQL_INVALID_HANDLE:
        throw;
    case SQL_ERROR:
        throw;
    default:
        break;
    }

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
    {
        throw;
    }

    if(SQL_SUCCESS!=SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
    {
        throw;
    }
    else
    {
        if(SQLFetch(sqlStatementhandle)==SQL_SUCCESS)
            SQLGetData(sqlStatementhandle, 1, SQL_C_LONG, &retornoCutHistoryId, 0, NULL);
    }
    return retornoCutHistoryId;

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);
}

void database::updateCutHistory(int cutHistoryId, int songRecognized, string dateTime)
{
    database::updateCutHistory(cutHistoryId, songRecognized, dateTime, false);
}

void database::updateCutHistory(int cutHistoryId, int songRecognized, string dateTime, bool silenceDetect)
{
    if  (cutHistoryId == 0)
        return;

    SQLCHAR retconstring[1024];
    int retornoCutHistoryId = 0;
    int silenceConvertToIntSql =  silenceDetect ? 1 : 0;

    string query = "UPDATE CutHistory SET  cuhi_dt_hora_da_processamento = '"+ dateTime +"', song_cd_oid = "+to_string(songRecognized)+" , cuhi_in_silencio = "+ to_string(silenceDetect) +" WHERE cuhi_cd_cuthistory = " + to_string(cutHistoryId);

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
    case SQL_INVALID_HANDLE:
        throw;
    case SQL_ERROR:
        throw;
    default:
        break;
    }

    if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_STMT, sqlConnectionhandle, &sqlStatementhandle))
        throw;

    if(SQL_SUCCESS!=SQLExecDirect(sqlStatementhandle, (SQLCHAR*)query.c_str(), SQL_NTS))
        throw;

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementhandle );
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvhandle);
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
