#ifndef SLICEPROCESS_H
#define SLICEPROCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <mutex>

#include "util.h"
#include "streamradio.h"
#include "rawdata.h"
#include "filedata.h"

using namespace std;

class SliceProcess
{
    public:
        enum enumSliceProcess
        {
            STOP = 0,
            RUN,
            ERROR
        };

        SliceProcess();
        virtual ~SliceProcess();

        void thrProcessa();
        int getStatus();


        int mrOn;
        bool svFP;
        string ipRecognition;
        string portRecognition;
        string mySqlConnString;

        string cutFolder;

        int idRadio;

        mutex* MutexAccess;

        vector<Filter> *Filters;

        StreamRadio* objRadio;
    protected:
    private:
        bool stopThread;

        int idSlice;
        int Status;

        AVCodecContext* cdc_ctx_in;

        RAWData* objRawData;
        FileData* objFileData;

        thread* objThreadRawParser;
        thread* objThreadArqParser;

        string getDate();
        string getHour();
        string getDateTime();
        string getSaveCutDir();

//        AVFrame* inFrame;

        vector<vector<vector<uint8_t>>> Packets;
        vector<vector <uint8_t>> aux;
};

#endif // SLICEPROCESS_H
