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
    ThreadCapture* objCapture;
    boost::thread* objThread;
};


class ThreadPool
{
    public:

        int ppp = 0;
        /** Default constructor */
        ThreadPool();
        /** Default destructor */
        virtual ~ThreadPool();

        string slqConnString;
        string ipRecognition;
        string portRecognition;

        vector<Filter> Filters;

        void addThreads(string uriRadio, int id);
        void stopThread(int id);

        void TesteThread();
    protected:
    private:
        //vector<ctrlThread*> ctrlThreads;
        map<unsigned int, ctrlThread*> ctrlThreads;
};

#endif // THREADPOOL_H
