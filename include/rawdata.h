#ifndef RAWDATA_H
#define RAWDATA_H

#include <thread>
#include <mutex>

#include <mir/fingerprint.h>
#include <mir/tcpclient.h>
#include <mir/database_mysql.h>

#include "util.h"
#include "exceptionmir.h"
#include "parser.h"

#define RAW_SAMPLE_RATE 11025
#define SOCKET_TIMEOUT  20000          // milisegundos

class RAWData : public Parser
{
    public:
        /** Default constructor */
        RAWData();
        /** Default destructor */
        virtual ~RAWData();

        void Execute();

        vector<Filter> *Filters;
        int mrOn;
        bool svFP;
        string ipRecognition;
        string portRecognition;
        string mySqlConnString;
        int32_t idSlice;
        mutex* MutexAccess;

    protected:
        virtual void EndResample();
    private:
        int32_t freq;

        Database_MySql* objMySql;

        // vetor para armazenar os dados para obter o fingerprint
        vector <uint8_t> binOutput;

        unsigned int* CreateFingerPrint(vector <uint8_t> Data, unsigned int* FingerPrintSize, bool mltFFT);
};

#endif // RAWDATA_H
