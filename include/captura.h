#ifndef CAPTURA_H
#define CAPTURA_H

/**< controle de versao */
#define prgVersion "1.0.0.1"

#include <iostream>
#include <fstream>
#include <signal.h>

extern "C"
{
    /** inclusão dos headers FFMPEG */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <mir/filter.h>
#include <mir/database_sql.h>
#include <mir/exceptionclass.h>

#include "util.h"
#include "configuration.h"
#include "threadpool.h"
#include "logger.h"

class Captura
{
    public:
        Captura();
        virtual ~Captura();
        void initLoop();
    protected:
    private:
        void writeFileStream();
        int readFileStream();
        int loadStream();
        void init();
        static void signal_callback_handler(int signum);

        // objeto do banco de dados
        Database_SQL* db;
        // configurações gerais da aplicação
        Configuration* config;
        // lista de streams
        vector<UrlStream*> urlStream;
        // filtro utilizado pelo fingerprint
        vector<Filter> Filters;

        ThreadPool* objThreadPool;
};

#endif
