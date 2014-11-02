#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <chrono>

#include "util.h"
#include "threadcapture.h"

struct ctrlThread
{
    bool isStop;
    int idThread;
    string uriRadio;
    ThreadCapture *objCapture;
    thread *objThread;
};


class ThreadPool
{
    public:
        /** Default constructor */
        ThreadPool(int mrOn, bool svFP, string ipRecognition, string portRecognition, string ipResult, string portResult,
                   string sqlConnString, string cutFolder, vector<Filter> *Filters);
        /** Default destructor */
        virtual ~ThreadPool();

        void addThreads(string uriRadio, int idRadio);
        void stopThread(int idRadio);
        string getUrlRadio(int idRadio);
        vector <int32_t> getActiveThread();
    protected:
    private:
        int mrOn;
        bool svFP;
        string ipRecognition;
        string portRecognition;
        string ipResult;
        string portResult;
        string mySqlConnString;

        string cutFolder;

        vector<Filter> *Filters;

        map<unsigned int, ctrlThread*> ctrlThreads;

        mutex MutexAccess;
};

#endif // THREADPOOL_H
