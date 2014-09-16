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
        SliceProcess(string ipRecognition, string portRecognition, string sqlConnString,
                     string cutFolder, int idRadio, vector<Filter> *Filters, StreamRadio* objRadio);
        virtual ~SliceProcess();

        void thrProcessa();
    protected:
    private:
        bool stopThread;

        string ipRecognition;
        string portRecognition;
        string sqlConnString;

        string cutFolder;

        int status;
        int idRadio;
        int idSlice;

        StreamRadio* objRadio;

        vector<Filter> *Filters;

        boost::thread* objThreadRawParser;
        boost::thread* objThreadArqParser;

        string getDate();
        string getHour();
        string getDateTime();
        string getSaveCutDir();
};

#endif // SLICEPROCESS_H
