#ifndef RAWDATA_H
#define RAWDATA_H

#include <mir/fingerprint.h>
#include <mir/tcpclient.h>

#include "exceptionmir.h"
#include "parser.h"

class RAWData : public Parser
{
    public:
        /** Default constructor */
        RAWData(string fileName, uint64_t channelLayoutIn, int sampleRateIn, int bitRateIn,
                AVSampleFormat sampleFormatIn, int nbSamplesIn, int nbChannelIn,
                vector<Filter> *Filters, string ipRecognition, string portRecognition,
                int32_t idRadio, int32_t idSlice);
        /** Default destructor */
        virtual ~RAWData();

        void Execute();
    protected:
        virtual void initObject();
        virtual void EndResample();
    private:
        vector<Filter> *Filters;
        string ipRecognition;
        string portRecognition;
        int32_t freq;
        int32_t idRadio;
        int32_t idSlice;

        // vetor para armazenar os dados para obter o fingerprint
        vector <uint8_t> binOutput;

        unsigned int* CreateFingerPrint(vector <uint8_t> Data, unsigned int* FingerPrintSize, bool mltFFT);
};

#endif // RAWDATA_H
