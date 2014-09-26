#ifndef DATABASE_MYSQL_H
#define DATABASE_MYSQL_H

#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <string>

//Referencia para o MySQL
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

//Referencia para manipulaço da connectionString
#include <boost/algorithm/string.hpp>


using namespace std;

struct Recorte
{
    int id;

    uint8_t * fingerPrint ;
    int fingerPrintSize;
    int bits;

    string datetime;

    string path;
    int pathSize;

    int song;
    int radio;

    int qtdProcessed;
    bool sent;
    bool error;
};

struct membuf: std::streambuf {
    membuf(char* base, std::ptrdiff_t n) {
        this->setg(base, base, base + n);
    }
};

class Database_MySql
{

public:
    Database_MySql(string connectionString);
    virtual ~Database_MySql();

    /** \brief
    * Insere o recorte em um banco MYSQL local para contingencia caso algum erro ocorra durante o percurso do recorte.
    *
    * \param fingerPrint           - FingerPrint
    * \param radio          - ID da radio.
    * \param bits           - Bits do fingerPrint que será usado para enviar para o MRServer
    * \param datetime       - Use o getDateSqlString para pegar a data atual com o formato correto.
    * \param pathMp3        - Caminho do arquivo MP3.
    *
    * \exception
    */
    int insertRecorte(uint8_t* fingerPrint , int radio, int bits,string datetime,  string pathMp3);

    /** \brief
    * Atualiza a quantidade de vezes que já processou o recorte.
    *
    * \param idRecorte      - ID do recorte, para pegar esse ID voce deve primeiro inseri-lo no banco com o método insertRecorte().
    *
    * \exception
    */
    void updateRecorteQtdProcessed(int idRecorte);

    /** \brief
    * Atualiza o resultado da música que o MRSERVER retornou no MYSQL.
    *
    * \param handletype    - Todos os métodos da api com sql tem retorno em int, esse será o parametro para passarno handleType.
    * \param handle        - SQLHANDLE da conexão com sql.
    *
    * \exception
    */
    void updateRecorteSong(int idRecorte, int songResult);

    /** \brief
    * Deleta o recorte do MYSQL.
    *
    * \param handletype    - Todos os métodos da api com sql tem retorno em int, esse será o parametro para passarno handleType.
    *
    * \exception
    */
    void deleteRecorteByID(int idRecorte);

    /** \brief
    * Pega o recorte que houve erro na hora de comunicação com MRServer, ou seja no plano de contigencia deverá mandar para
    * o MRServer daí então dar continuidade ao caminho do recorte.
    *
    * \exception
    */
    Recorte* selectRecorteErrorMRServer();

    /** \brief
    * Pega o que houve erro na hora de comunicação com MRServer, ou seja no plano de contigencia deverá mandar para
    * o MRServer daí então dar continuidade ao caminho do recorte.
    *
    *\param retorna os TOP primeiros.
    *
    * \exception
    */
    vector<Recorte*> selectRecorteErrorMRServer(int topFirst);

    /** \brief
    * Pega o recorte onde houve erro na hora da comunicação do SQLServer, ou seja esse método será usado
    * no plano de contigencia deverá apenas inserir no banco de dados TEMP.
    *
    * \exception
    */
    Recorte * selectRecorteErrorSqlServer();

    /** \brief
    * Pega o primeiro recorte onde houve erro na hora da comunicação do SQLServer, ou seja esse método será usado
    * no plano de contigencia deverá apenas inserir no banco de dados TEMP.
    *
    *\param retorna os TOP primeiros.
    *
    * \exception
    */
    vector<Recorte*> selectRecorteErrorSqlServer(int topFirst);

    /** \brief
    * Método para abrir conexão com MySql.
    *
    * \exception
    */
    void open();

    /** \brief
    * Método para abrir conexão com MySql.
    *
    * \exception
    */
    void close();

    /** \brief
    * Pega data e hora atual em string com o padrão do SQL server (yyyy-MM-dd HH:mm:ss).
    *
    * \return               -  Retorno da data atual.
    */
    string getDateSqlString();

protected:
private:

    /** \brief
    * Método que transformar o ConnectionString em usuário, Source, Database, Password nos seus devidos atribuitos.
    *
    *\param connectionString.
    *
    * \exception
    */
    void decodeConnectionString(string connectionString);

    /** \brief
    * Método que executa um Statment no MySql.
    *
    *\param PreparedStatment.
    *
    * \exception
    */
    sql::ResultSet* executeStatment(sql::PreparedStatement *pstmt);

      /** \brief
    * Método que executa um Statment no MySql.
    *
    *\param Statment.
    *\param Query
    *
    * \exception
    */
    sql::ResultSet* executeStatment(sql::Statement *pstmt, string query);

    string User;
    string Source;
    string DataBase;
    string Password;
    bool IsOpen;
    sql::Connection *ConnectionMySql;
    sql::Driver *DriverMySql;

};

#endif // DATABASE_MYSQL_H
