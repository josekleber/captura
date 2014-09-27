#ifndef RAWDATA_H
#define RAWDATA_H

#include <mir/fingerprint.h>
#include <mir/tcpclient.h>
#include <mir/database_mysql.h>

#include "exceptionmir.h"
#include "parser.h"

#define RAW_SAMPLE_RATE 11025

class RAWData : public Parser
{
    public:
        /** Default constructor */
        RAWData();
        RAWData(string fileName, uint64_t channelLayoutIn, int sampleRateIn, int bitRateIn,
                AVSampleFormat sampleFormatIn, int nbSamplesIn, int nbChannelIn,
                vector<Filter> *Filters, int mrOn, bool svFP, string ipRecognition, string portRecognition,
                int32_t idRadio, int32_t idSlice);
        /** Default destructor */
        virtual ~RAWData();

        void Execute();

        vector<Filter> *Filters;
        int mrOn;
        bool svFP;
        string ipRecognition;
        string portRecognition;
        string mySqlConnString;
        int32_t idRadio;
        int32_t idSlice;

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
