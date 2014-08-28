#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdlib.h>
#include <stdio.h>
#include <mir/filter.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#include "streamradio.h"
#include "parser.h"

using namespace std;

/** \brief
*  Classe utilizado para criar a thread principal, onde conecta com a radio, captura um recorte,
*  transforma em fingerprint, envia para o servidor de reconhecimento e atualiza a base de dados.
 */
class ThreadCapture
{
    public:
        ThreadCapture();
        virtual ~ThreadCapture();

        string uriRadio;

        string ipRecognition;
        string portRecognition;
        string sqlConnString;

        string cutFolder;

        int status;
        int idThread;   // igual a idRadio

        vector<Filter> *Filters;

        /** \brief Thread de um canal. cria o objeto MRServer e acerta os parametros
         *
         * \param tmp void*
         * \return void*
         *
         */
        void init();
        void thrRun();
        void thrClose();
    protected:
    private:
        bool stopThread;

        boost::thread* objThreadRadio;
        boost::thread* objThreadRawParser;
        boost::thread* objThreadM4aParser;
        Parser* objParser;
        StreamRadio* objRadio;
};

#endif // CAPTURE_H
