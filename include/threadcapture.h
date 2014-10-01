#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <mutex>

#include <mir/filter.h>

#include "util.h"
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
        ThreadCapture();
        ThreadCapture(int mrOn, bool svFP, string ipRecognition, string portRecognition,
                      string mySqlConnString, int idThread, string uriRadio,
                      vector<Filter> *Filters, string cutFolder);
        virtual ~ThreadCapture();

        int status;

        void thrRun();

        int mrOn;
        bool svFP;
        string ipRecognition;
        string portRecognition;
        string mySqlConnString;

        mutex* MutexAccess;

        string uriRadio;
        int idThread;   // igual a idRadio
        vector<Filter> *Filters;
        string cutFolder;
    protected:
    private:
        bool stopThread;

        StreamRadio* objRadio;
        SliceProcess* objSlice;

        thread* objThreadRadio;
        thread* objThreadProcessa;
};

#endif // CAPTURE_H
