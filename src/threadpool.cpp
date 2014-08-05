#include "threadpool.h"

using namespace std;

ThreadPool::ThreadPool()
{
    //ctor
}

ThreadPool::~ThreadPool()
{
    //dtor
}

void ThreadPool::addThreads(string uriRadio, int id)
{
    ctrlThread* objThreadControl = new ctrlThread;
    objThreadControl->isStop = true;
    objThreadControl->idThread = id;

    objThreadControl->objCapture = new ThreadCapture;
    objThreadControl->objCapture->ipRecognition = ipRecognition;
    objThreadControl->objCapture->portRecognition = portRecognition;
    objThreadControl->objCapture->sqlConnString = sqlConnString;
    objThreadControl->objCapture->uriRadio = uriRadio;
    objThreadControl->objCapture->idThread = id;
    objThreadControl->objCapture->Filters = Filters;
    objThreadControl->objCapture->cutFolder = cutFolder;

    objThreadControl->objThread = new boost::thread(boost::bind(&ThreadCapture::thrRun, objThreadControl->objCapture));
    objThreadControl->isStop = false;

    ctrlThreads[id] = objThreadControl;
}

void ThreadPool::stopThread(int id)
{
    ctrlThread* objThreadControl = ctrlThreads[id];
    objThreadControl->objCapture->thrClose();
}

void ThreadPool::TesteThread(int cnt)
{
    for (int i = 1; i <= cnt; i++)
        this->addThreads("Thread numero " + std::to_string(i), i);

    for (int i = cnt; i > 0; i--)
    {
        sleep(2);
        this->stopThread(i);
    }
}
