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
#include <queue>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

/**< caractere de controle de tela utilizando o caractere esc (\033). So funciona para telas que aceitam esse controle */
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

/**< define dos tipos de mensagens. Podem ser utilizadas em conjuto entre elas com o operador OR */
#define MR_LOG_MESSAGE      1
#define MR_LOG_DEBUG        2
#define MR_LOG_ERROR        4
#define MR_LOG_WARNING      8
#define MR_LOG_CRITICAL     16
#define MR_LOG_SYSTEM       32
#define MR_LOG_AUDIT        64
/**< utilizando esse em conjunto com os acima, faz com que o log nao apresente a data e hora no inicio da linha */
#define MR_LOG_NO_DATATIME  128

/**< indica o tamanho e quantidades maximas de arquivos de log */
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

        LogClass(bool flFile, bool flScreen, bool flMsg, bool flDebug);
        virtual ~LogClass();
        void mr_printf(unsigned int type, int ident, int Lin, int Col, const char *fmt, ...);
        void mr_printf(unsigned int type, int ident, const char *fmt, ...);
        char *getDate(bool withSep);
        uintmax_t fileSize(void);
    protected:
    private:
        bool tmPause;
        char LogName[40];
        char strPosicao[30];
        static bool decSort(string a, string b);

        void delLog(void);
        void OpenMRLog(void);
};

#endif // LOGCLASS_H
