#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <chrono>

#include "threadcapture.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

struct ctrlThread
{
    bool isStop;
    int idThread;
    string uriRadio;
    ThreadCapture *objCapture;
    boost::thread *objThread;
};


class ThreadPool
{
    public:
        /** Default constructor */
        ThreadPool(int mrOn, bool svFP, string ipRecognition, string portRecognition,
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
        string sqlConnString;

        string cutFolder;

        vector<Filter> *Filters;

        map<unsigned int, ctrlThread*> ctrlThreads;
};

#endif // THREADPOOL_H
