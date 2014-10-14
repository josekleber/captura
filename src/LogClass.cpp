#include "LogClass.h"

LogClass::LogClass(bool flFile, bool flScreen, bool flMsg, bool flDebug)
{
    toFile = flFile;
    toScreen = flScreen;

    onMsg = flMsg;
    onDebug = flDebug;

/**< modo estatistica desligado e posicao default das mensagens fixas */
    flStat = false;
    msgLin = 18;
    errLin = 22;

    tmPause = false;

    strPosicao[0] = 0;

    OpenMRLog();
}

LogClass::~LogClass()
{
    //dtor
    fclose(log);
}

bool LogClass::decSort(string a, string b)
{
    return (a > b);
}

void LogClass::mr_printf(unsigned int type, int ident, int Lin, int Col, const char *fmt, ...)
{
    char strAux[256];

    va_list args;
    va_start(args, fmt);

    vsprintf(strAux, fmt, args);

    sprintf(strPosicao, "\033[%d;%dH", Lin, Col);
    mr_printf(type, ident, strAux);
}

void LogClass::mr_printf(unsigned int type, int ident, const char *fmt, ...)
{
    char strLog[256];
    char strLog1[256];
    char strTipo[6];

    va_list args;
    va_start(args, fmt);
    vsprintf(strLog, fmt, args);

//!< escolhe o cabecario da mensagem
    strTipo[0] = 0;

    if ((type & MR_LOG_SYSTEM) != 0)
        sprintf(strTipo, "Sys  ");

    if ((type & MR_LOG_AUDIT) != 0)
        sprintf(strTipo, "Audit");

    if ((type & MR_LOG_MESSAGE) != 0)
        sprintf(strTipo, "Msg  ");

    if ((type & MR_LOG_DEBUG) != 0)
        sprintf(strTipo, "Debug");

    if ((type & (MR_LOG_ERROR | MR_LOG_WARNING | MR_LOG_CRITICAL)) != 0)
        sprintf(strTipo, "Erro ");

//!< coloca a data/hora, se necessario
    if ((type & MR_LOG_NO_DATATIME) == 0)
        sprintf(strLog1, "[%s] %s : [%5d] %s", getDate(true).c_str(), strTipo, ident, strLog);
    else
        sprintf(strLog1, "[%5d] %s", ident, strLog);

//!< se for tipo auditoria, escreve no arquivo
    if ((type & MR_LOG_AUDIT) != 0)
    {
        printf("%s", strLog1);

        fprintf(log, "%s", strLog1);
        fflush(log);
    }

//!< se for mensagem de erro, escreve no arquivo
    if ((type & (MR_LOG_ERROR | MR_LOG_WARNING | MR_LOG_CRITICAL)) != 0)
    {
        if (fileSize() >= MR_LOG_MAXSIZE)
        {
            fclose(log);
            OpenMRLog();
        }

        fprintf(log, "%s", strLog1);
        fflush(log);
    }

//!< processa mensagem do tipo tela. aqui temos a verificacao de posicionamento na tela
//!< se for tipo erro, muda a cor da fonte
    if (toScreen || ((type & MR_LOG_SYSTEM) != 0))
    {
        char strPos[30];

        if ((onMsg && ((type & MR_LOG_MESSAGE) != 0))
        	|| ((type & MR_LOG_SYSTEM) != 0)
        	|| (onDebug && ((type & MR_LOG_DEBUG) != 0)))
        {
            if (flStat && (strPosicao[0] == '\0'))
                sprintf(strPos, "\033[%d;1H\033[2K", msgLin);
            else
                sprintf(strPos, "%s", strPosicao);

        	printf(MR_LOG_RESET"%s%s", strPos, strLog1);
        }

        if ((type & (MR_LOG_ERROR | MR_LOG_WARNING | MR_LOG_CRITICAL)) != 0)
        {
            if (flStat && (strPosicao[0] == '\0'))
                sprintf(strPos, "\033[%d;1H\033[2K", errLin);
            else
                sprintf(strPos, "%s", strPosicao);

        	printf(MR_LOG_RED "%s%s" MR_LOG_RESET, strPos, strLog1);
        }

        strPosicao[0] = 0;
    }

//!< processa mensagem do tipo arquivo.
//!< verifica se o arquivo e maior que o limite.
//!< se a quantidade de logs for maior que o limite, apaga os mais antigos
    if (toFile)
    {
        if ((onMsg && ((type & MR_LOG_MESSAGE) != 0))
        	|| (onDebug && ((type & MR_LOG_DEBUG) != 0)))
        {
		    if (fileSize() >= MR_LOG_MAXSIZE)
		    {
		        fclose(log);
		        OpenMRLog();
		    }

		    fprintf(log, "%s", strLog1);
		    fflush(log);
        }
    }
}

string LogClass::getDate(bool withSep)
{
    struct tm *DtHr;
    time_t t;
    char strAux[20];

    t = time(NULL);
    DtHr = localtime(&t);

    if (withSep)
        sprintf(strAux, "%d/%02d/%02d %02d:%02d:%02d", DtHr->tm_year + 1900, DtHr->tm_mon + 1, DtHr->tm_mday, DtHr->tm_hour, DtHr->tm_min, DtHr->tm_sec);
    else
        sprintf(strAux, "%d%02d%02d_%02d%02d%02d", DtHr->tm_year + 1900, DtHr->tm_mon + 1, DtHr->tm_mday, DtHr->tm_hour, DtHr->tm_min, DtHr->tm_sec);

    return (string)strAux;
}

void LogClass::delLog(void)
{
    DIR *dir;
    struct dirent *arq;
    unsigned char isFile = 0x08;
    vector <string> lstLogs;

//!< se nao existir a pasta log, cria
    if ((dir = opendir("./log")) == NULL)
    {
        mkdir("log", 0777);
    }
    else
    {
//!< armazena todos os arquivos na lista
        while ((arq = readdir(dir)))
        {
            if (arq->d_type == isFile)
            {
                char aux[150];
                sprintf(aux, "./log/%s", arq->d_name);
                lstLogs.push_back(aux);
            }
        }

        closedir(dir);

//!< ordena descrescente
        sort(lstLogs.begin(), lstLogs.end(), &decSort);

//!< apaga os mais antigos
        if (lstLogs.size() > MR_LOG_MAXFILE)
        {
            int cnt = 0;
            for (vector<string>::iterator it = lstLogs.begin(); it != lstLogs.end(); it++)
            {
                cnt++;
                if (cnt > MR_LOG_MAXFILE)
                    remove(it->c_str());
            }
        }
    }
}

uintmax_t LogClass::fileSize(void)
{
    struct stat statbuf;

    if (stat(LogName, &statbuf) == -1)
    {
        return -1;
    }

    return (uintmax_t) statbuf.st_size;
}

void LogClass::OpenMRLog(void)
{
    DIR *dir;

    if ((dir = opendir("./log")) == NULL)
        mkdir("log", 0777);
    else
        closedir(dir);

    sprintf(LogName, "./log/mrserver_%s.log", getDate(false).c_str());
    log = fopen(LogName, "a");

    delLog();
}
