#ifndef SLICEPROCESS_H
#define SLICEPROCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <string>

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
        SliceProcess(int mrOn, bool svFP, string ipRecognition, string portRecognition, string sqlConnString,
                     string cutFolder, int idRadio, vector<Filter> *Filters, StreamRadio* objRadio);
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

        vector<Filter> *Filters;

        StreamRadio* objRadio;
    protected:
    private:
        bool stopThread;

        int idSlice;
        int Status;

        boost::thread* objThreadRawParser;
        boost::thread* objThreadArqParser;

        string getDate();
        string getHour();
        string getDateTime();
        string getSaveCutDir();
};

#endif // SLICEPROCESS_H
