#ifndef DATABASE_H
#define DATABASE_H
#include <time.h>
#include <iostream>
#include <list>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <vector>

using namespace std;

struct UrlStream
{
    int id;
    int radio;
    string urlStream;
};

/** \brief
* Classe que se comunica com o sql server.
*/
class database
{
    public:
        database();
        virtual ~database();

        /** \brief
        * Adiciona um parâmetro para a configuração das opções de conexão
        *
        * \param key    - parâmetro de configuração
        * \param value  - valor do parâmetro
        *
        * \return       - retorna um vetor de objectos com o ID da rádio e a URL do string do listener passado como paramêtro.
        * \exception
        */
        vector<UrlStream*>  getRadiosActive(string guidListener);

        /** \brief
        * Salva o status da rádio no banco SQL
        *
        * \param radioId    - ID da rádio que será marcado o status dela.
        * \param codeLog    - Código do log que será enviado para o banco.
        *
        * \exception
        */
        void saveLog(int radioId, string codeLog);

    protected:
        SQLCHAR* connectionString;
    private:

        /** \brief
        * Pega os detalhes do erro da conexão com o banco de dados.
        *
        * \param handletype    - Todos os métodos da api com sql tem retorno em int, esse será o parametro para passarno handleType.
        * \param handle        - SQLHANDLE da conexão com sql.
        *
        * \exception
        */
        void showError(unsigned int handletype, const SQLHANDLE& handle);

        /** \brief
        * Pega data e hora atual em string com o padrão do SQL server (yyyy-MM-dd HH:mm).
        *
        * \return               -  Retorno da data atual.
        */
        string getDateSqlString();

        /** \brief
        * Verificador de GUID
        *
        * \param guid           - GUID em string para verificar se a string passada é realmente um guid.
        * \return               - retorna um booleano informando se realmente é um guid.
        */
        bool isGuid(string guid);

        SQLHANDLE sqlEnvhandle;
        SQLHANDLE sqlConnectionhandle;
        SQLHANDLE sqlStatementhandle;
        SQLRETURN retCode;
};

#endif // DATABASE_H
