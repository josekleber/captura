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
    objThreadControl->objCapture->slqConnString = slqConnString;
    objThreadControl->objCapture->ipRecognition = ipRecognition;
    objThreadControl->objCapture->portRecognition = portRecognition;
    objThreadControl->objCapture->uriRadio = uriRadio;

//    objThreadControl->uriRadio = uriRadio;

    objThreadControl->objThread = new boost::thread(boost::bind(&ThreadCapture::thrRun, objThreadControl->objCapture));
//    objThreadControl->objThread->join();
    objThreadControl->isStop = false;

 //   ctrlThreads.push_back(objThreadControl);
    ctrlThreads[id] = objThreadControl;
}

void ThreadPool::stopThread(int id)
{
    ctrlThread* objThreadControl = ctrlThreads[id];
    objThreadControl->objCapture->thrClose();
}
