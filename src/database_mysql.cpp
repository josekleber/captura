#include "database_mysql.h"

Database_MySql::Database_MySql(string connectionString)
{
    this->IsOpen = false;
    decodeConnectionString(connectionString);
}

Database_MySql::~Database_MySql()
{
    if (!this->IsOpen)
        return;

    this->ConnectionMySql->close();
    delete ConnectionMySql;

}

int Database_MySql::insertRecorte(uint8_t* fingerPrint, int radio, int bits,string datetime,  string pathMp3)
{
    sql::ResultSet* result;
    sql::Statement* stmt ;
    sql::PreparedStatement *pstmt;
    int retorno;

    pstmt =this->ConnectionMySql->prepareStatement("insert into recorte (reco_tx_caminho, reco_cd_radio, reco_dt_data, reco_nm_bits, reco_bn_fingerprint) values (?,?,?,?,?); ");

    membuf sbuf((char*)fingerPrint, sizeof(fingerPrint));
    std::istream in(&sbuf);

    pstmt->setString(1, pathMp3);
    pstmt->setInt(2,radio);
    pstmt->setDateTime(3, datetime);
    pstmt->setInt(4, bits);
    pstmt->setBlob(5, &in);

    executeStatment(pstmt);

    stmt= this->ConnectionMySql->createStatement();

    result = executeStatment(stmt, "select last_insert_id()");

    if (result->next())
        retorno= result->getInt(1);

    delete pstmt;
    delete stmt;
    delete result;

    return retorno;// result->getInt(1);

}

void Database_MySql::updateRecorteQtdProcessed(int idRecorte)
{
    sql::PreparedStatement *pstmt = this->ConnectionMySql->prepareStatement("UPDATE recorte SET reco_nm_qtdprocessed = reco_nm_qtdprocessed + 1 WHERE reco_cd_recorte = ?");
    pstmt->setInt(1, idRecorte);
    executeStatment(pstmt);
    delete pstmt;
}

void Database_MySql::updateRecorteSong(int idRecorte, int songResult)
{
    sql::PreparedStatement *pstmt = this->ConnectionMySql->prepareStatement("UPDATE recorte SET reco_nm_qtd = ? WHERE reco_cd_recorte =?");
    pstmt->setInt(1, songResult);
    pstmt->setInt(2, idRecorte);
    executeStatment(pstmt);
    delete pstmt;
}

void Database_MySql::deleteRecorteByID(int idRecorte)
{
    sql::PreparedStatement *pstmt = this->ConnectionMySql->prepareStatement("DELETE FROM recorte WHERE reco_cd_recorte = ?");
    pstmt->setInt(1, idRecorte);
    executeStatment(pstmt);
    delete pstmt;
}

Recorte* Database_MySql::selectRecorteErrorMRServer()
{
    Recorte* recorteRetorno;
    istream* buffFinger;
    char * buff;

    //sql::PreparedStatement* pstmt = this->ConnectionMySql->prepareStatement("SELECT TOP 1 *, OCTET_LENGTH(reco_bn_fingerprint)  FROM recorte");
    sql::PreparedStatement* pstmt = this->ConnectionMySql->prepareStatement("SELECT arquivo, OCTET_LENGTH(arquivo) lenght  FROM teste LIMIT 1 ");
    sql::ResultSet * result  = executeStatment(pstmt);

    result->afterLast();
    result->previous();
//    recorteRetorno->id = result->getInt("reco_cd_recorte");
//    recorteRetorno->path = result->getString("");
//    recorteRetorno->error = result->getBoolean("");
    recorteRetorno->fingerPrintSize = result->getInt("lenght");
    buffFinger = result->getBlob("arquivo");
    buffFinger->read(buff, recorteRetorno->fingerPrintSize);
    recorteRetorno->fingerPrint = (uint8_t*)buff;

    return recorteRetorno;
}

vector<Recorte*> Database_MySql::selectRecorteErrorMRServer(int topFirst)
{


}

Recorte* Database_MySql::selectRecorteErrorSqlServer()
{

}

vector<Recorte*> Database_MySql::selectRecorteErrorSqlServer(int topFirst)
{

}

sql::ResultSet* Database_MySql::executeStatment(sql::PreparedStatement *pstmt)
{
    try
    {
        return pstmt->executeQuery();
    }
    catch (sql::SQLException &e)
    {
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line "  << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
}


sql::ResultSet* Database_MySql::executeStatment(sql::Statement *stmt, string query)
{
    try
    {
        return stmt->executeQuery(query);
    }
    catch (sql::SQLException &e)
    {
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
}


void Database_MySql::decodeConnectionString(string connectionString)
{
    std::vector<std::string> strs;
    boost::split(strs, connectionString, boost::is_any_of(";"));
    //cout << connectionString << endl;
    for(int i = 0; i < strs.size(); i++ )
    {
        vector<string> k;
        boost::split(k, strs[i], boost::is_any_of("="));

        if (k.size() > 1)
        {
            std::transform(k[0].begin(), k[0].end(), k[0].begin(), ::tolower);

            if (k[0] == "server")
            {
                this->Source ="tcp://" + k[1] + ":3306";
                continue;
            }
            if (k[0] == "database")
            {
                this->DataBase = k[1];
                continue;
            }
            if (k[0] == "uid")
            {
                this->User = k[1];
                continue;
            }
            if (k[0] == "pwd")
            {
                this->Password= k[1];
                continue;
            }
        }
    }
}

void Database_MySql::open()
{
    if(!this->IsOpen)
    {
        try
        {
            this->IsOpen = true;
            this->DriverMySql = get_driver_instance();
            this->ConnectionMySql = this->DriverMySql->connect(Source, User, Password);
            this->ConnectionMySql->setSchema(DataBase);
        }
        catch (sql::SQLException &e)
        {
            cout << "# ERR: SQLException in " << __FILE__ << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
            cout << "# ERR: " << e.what();
            cout << " (MySQL error code: " << e.getErrorCode();
            cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        }
    }
}

string Database_MySql::getDateSqlString()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [18];

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,80,"%F %T",timeinfo);
    return string(buffer);
}

void Database_MySql::close()
{

    if(this->IsOpen)
    {
        try
        {
            this->IsOpen = false;
            this->ConnectionMySql->close();
            delete ConnectionMySql;
        }
        catch (sql::SQLException &e)
        {
            cout << "# ERR: SQLException in " << __FILE__<< endl;
            cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
            cout << "# ERR: " << e.what() <<endl;
            cout << " (MySQL error code: " << e.getErrorCode() <<endl;
            cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        }
    }
}
