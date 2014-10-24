/**
 *
 * Author: Nelson Nagamine
 * 2013
 *
 */

#ifndef LOGCLASS_H
#define LOGCLASS_H

#include <syslog.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <queue>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

//!< caractere de controle de tela utilizando o caractere esc (\033). So funciona para telas que aceitam esse controle
#define MR_LOG_CLEAR        "\033[2J\033[1;1H"       /* Apaga a tela */
#define MR_LOG_RESET        "\033[0m"                /* reseta o controle */
#define MR_LOG_BLACK        "\033[30m"               /* Black */
#define MR_LOG_RED          "\033[31m"               /* Red */
#define MR_LOG_GREEN        "\033[32m"               /* Green */
#define MR_LOG_YELLOW       "\033[33m"               /* Yellow */
#define MR_LOG_BLUE         "\033[34m"               /* Blue */
#define MR_LOG_MAGENTA      "\033[35m"               /* Magenta */
#define MR_LOG_CYAN         "\033[36m"               /* Cyan */
#define MR_LOG_WHITE        "\033[37m"               /* White */
#define MR_LOG_BOLDBLACK    "\033[1m\033[30m"        /* Bold Black */
#define MR_LOG_BOLDRED      "\033[1m\033[31m"        /* Bold Red */
#define MR_LOG_BOLDGREEN    "\033[1m\033[32m"        /* Bold Green */
#define MR_LOG_BOLDYELLOW   "\033[1m\033[33m"        /* Bold Yellow */
#define MR_LOG_BOLDBLUE     "\033[1m\033[34m"        /* Bold Blue */
#define MR_LOG_BOLDMAGENTA  "\033[1m\033[35m"        /* Bold Magenta */
#define MR_LOG_BOLDCYAN     "\033[1m\033[36m"        /* Bold Cyan */
#define MR_LOG_BOLDWHITE    "\033[1m\033[37m"        /* Bold White */

//!< define dos tipos de mensagens. Podem ser utilizadas em conjuto entre elas com o operador OR
#define MR_LOG_MESSAGE      1
#define MR_LOG_DEBUG        2
#define MR_LOG_ERROR        4
#define MR_LOG_WARNING      8
#define MR_LOG_CRITICAL     16
#define MR_LOG_SYSTEM       32
#define MR_LOG_AUDIT        64
//!< utilizando esse em conjunto com os acima, faz com que o log nao apresente a data e hora no inicio da linha
#define MR_LOG_NO_DATATIME  128

//!< indica o tamanho e quantidades maximas de arquivos de log
#define MR_LOG_MAXSIZE 10485760               /* 10 mb */
#define MR_LOG_MAXFILE 10

class LogClass
{
    public:
        // flags das mensagens habilitadas
        bool onMsg;
        bool onDebug;

        // flags do destino da mensagem
        bool toFile;
        bool toScreen;

        // variaveis para impressao na tela em posicao fixa
        bool flStat;
        int msgLin, errLin;

        FILE* log;

/** \brief Construtor da classe. acerta os flags de saida.
 *
 * \param flFile bool
 * \param flScreen bool
 * \param flMsg bool
 * \param flDebug bool
 *
 */
        LogClass(bool flFile, bool flScreen, bool flMsg, bool flDebug);
        virtual ~LogClass();
/** \brief imprime comentario no log. caso uma das saidas for na tela, posiciona a mensagem na linha/coluna
 *
 * \param type unsigned int
 * \param Lin int
 * \param Col int
 * \param fmt const char*
 * \param ...
 * \return void
 *
 */
        void mr_printf(unsigned int type, int ident, int Lin, int Col, const char *fmt, ...);
/** \brief imprime comentario no log. o uso dele e o mesmo que o printf (formatacao)
 *
 * \param type unsigned int
 * \param fmt const char*
 * \param ...
 * \return void
 *
 */
        void mr_printf(unsigned int type, int ident, const char *fmt, ...);
/** \brief devolve a data/hora atual.
 *
 * \param withSep bool com ou sem separadores
 * \return char*
 *
 */
        string getDate(bool withSep);
/** \brief pega o tamanho do arquivo
 *
 * \param void
 * \return uintmax_t
 *
 */
        uintmax_t fileSize(void);
    protected:
    private:
        bool tmPause;
        char LogName[40];
        char strPosicao[30];

/** \brief rotina para ordenar, utilizado no controle de quantidade de arquivos log.
 *
 * \param a string
 * \param b string
 * \return bool
 *
 */
        static bool decSort(string a, string b);

/** \brief apaga logs antigos
 *
 * \param void
 * \return void
 *
 */
        void delLog(void);
/** \brief abre um novo arquivo de log. verifica se a pasta existe bem como a quantidade de arquivos
 *
 * \param void
 * \return void
 *
 */
        void OpenMRLog(void);
};

#endif // LOGCLASS_H
