#ifndef DATABASE_H
#define DATABASE_H
#include "exceptionmir.h"
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
    string url;
};
enum CodeLog
{
    CAPTURE_BEGIN,
    CAPTURE_ERROR ,
    CAPTURE_SUCESS,
    CAPTURE_END
};

struct InvalidArgumentExecption : virtual BaseException {};

struct SqlServerException : virtual BaseException {};

/** \brief
* Classe que se comunica com o sql server.
*/
class Database
{
    public:
        Database();
        virtual ~Database();

        /** \brief
        * Pega data e hora atual em string com o padrão do SQL server (yyyy-MM-dd HH:mm:ss).
        *
        * \return               -  Retorno da data atual.
        */
        string getDateSqlString();

        /** \brief
        * Adiciona um parâmetro para a configuração das opções de conexão
        *
        * \param key            - parâmetro de configuração
        * \param value          - valor do parâmetro
        *
        * \return               - retorna um vetor de objectos com o ID da rádio e a URL do string do listener passado como paramêtro.
        * \exception
        */
        vector<UrlStream*>  getRadiosActive(string guidListener);

        /** \brief
        * Salva o status da rádio no banco SQL
        *
        * \param radioId        - ID da rádio que será marcado o status dela.
        * \param codeLog        - Código do log que será enviado para o banco:
        *
        *                           MOT001 - Inicio da captura
        *                           MOT007 - Capturado com sucesso
        *                           MOT005 - Erro na captura
        *                           MOT002 - Parada na captura
        *
        * \exception
        */
        void saveLog(int radioId, CodeLog codeLog);

        /** \brief
        * Grava histórico do recorte da captura no SQL.
        *
        * \param radio          - ID da rádio que esta capturando.
        * \param path           - Caminhod o arquivo onde será gravado para auditoria.
        * \param dateTime       - Hora que foi efeutada a captura, parametro aceito apenas no seguinte formato:
        *                            yyyy-MM-dd HH:mm:ss
        *                           OBS: Use o método getDateSqlString() que retorna o formato correto da hora atual.
        *
        * \return               - Retorna o ID (int) do registro do recorte na tabela CutHistory, será utilizado ele
        *                       na hora de chamar o método updateCutHistory();
        * \exception
        */
        int insertCutHistory(int radio, string dateTime , string path);

        /** \brief
        * Atualiza registro banco de dados do resultado do servidor de reconhecimento.
        *
        * \param cutHistoryId   - ID pelo método insertCutHistory.
        * \param songRecognized - ID da música reconhecida que retornou do MRServer.
        * \param dateTime       - Hora que foi efeutada o processamento, parametro aceito apenas no seguinte formato:
        *                               yyyy-MM-dd HH:mm:ss
        *                            OBS: Use o método getDateSqlString() que retorna o formato correto da hora atual.
        * \param silenceDetect  - Se o recorte captura contem silencio.
        *
        * \return               - Retorna o ID (int) do registro do recorte na tabela CutHistory, será utilizado ele
        *                       na hora de chamar o método updateCutHistory();
        * \exception
        */
        void updateCutHistory(int cutHistoryId, int songRecognized, string dateTime, bool silenceDetect);

        /** \brief
        * Sobrecarga do método de atualizar informação do histórico, porém sem o paramêtro de detecção do silencio.
        * "Atualiza registro banco de dados do resultado do servidor de reconhecimento."
        *
        * \param cutHistoryId   - ID pelo método insertCutHistory.
        * \param songRecognized - ID da música reconhecida que retornou do MRServer.
        * \param dateTime       - Hora que foi efeutada o processamento, parametro aceito apenas no seguinte formato:
        *                               yyyy-MM-dd HH:mm:ss
        *                            OBS: Use o método getDateSqlString() que retorna o formato correto da hora atual.
        *
        * \return               - Retorna o ID (int) do registro do recorte na tabela CutHistory, será utilizado ele
        *                       na hora de chamar o método updateCutHistory();
        * \exception
        */
        void updateCutHistory(int cutHistoryId, int songRecognized, string dateTime);

        /** \brief
        * Pega os detalhes do erro da conexão com o banco de dados.
        *
        * \param handletype    - Todos os métodos da api com sql tem retorno em int, esse será o parametro para passarno handleType.
        * \param handle        - SQLHANDLE da conexão com sql.
        *
        * \exception
        */
        void logError(unsigned int handletype, const SQLHANDLE& handle);

    protected:
        SQLCHAR* connectionString;
    private:
        SQLHENV sqlEnvhandle;
        SQLHANDLE sqlConnectionhandle;
        SQLHSTMT sqlStatementhandle;
        SQLRETURN retCode;
};

#endif // DATABASE_H
