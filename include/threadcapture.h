#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdlib.h>
#include <stdio.h>
#include <mir/filter.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#include "streamradio.h"
#include "sliceprocess.h"

using namespace std;

/** \brief
*  Classe utilizado para criar a thread principal, onde conecta com a radio, captura um recorte,
*  transforma em fingerprint, envia para o servidor de reconhecimento e atualiza a base de dados.
 */
class ThreadCapture
{
    public:
        ThreadCapture(int mrOn, bool svFP, string ipRecognition, string portRecognition,
                      string sqlConnString, int idThread, string uriRadio,
                      vector<Filter> *Filters, string cutFolder);
        virtual ~ThreadCapture();

        int status;

        void thrRun();
    protected:
    private:
        int mrOn;
        bool svFP;
        string ipRecognition;
        string portRecognition;
        string sqlConnString;

        string uriRadio;
        int idThread;   // igual a idRadio
        vector<Filter> *Filters;
        string cutFolder;

        bool stopThread;

        StreamRadio* objRadio;
        SliceProcess* objSlice;

        boost::thread* objThreadRadio;
        boost::thread* objThreadProcessa;
};

#endif // CAPTURE_H
