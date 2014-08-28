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
    ctrlThread* objThreadControl;

    try
    {
        objThreadControl = new ctrlThread;
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

        objThreadControl->objCapture->init();
    }
    catch(...)
    {
        throw;
    }

    try
    {
        objThreadControl->objThread = new boost::thread(boost::bind(&ThreadCapture::thrRun, objThreadControl->objCapture));
        objThreadControl->isStop = false;

        if (objThreadControl->objCapture->status == 0)
            ctrlThreads[id] = objThreadControl;
    }
    catch(...)
    {
        throw;
    }
}

void ThreadPool::stopThread(int id)
{

    std::map<unsigned int, ctrlThread*>::iterator itAux;
    ctrlThread* objThreadControl;

    itAux = ctrlThreads.find(id);

    if (itAux != ctrlThreads.end())
    {
        objThreadControl = itAux->second;
        objThreadControl->objCapture->thrClose();
    }
}

string ThreadPool::getUrlRadio(int id)
{
    std::map<unsigned int, ctrlThread*>::iterator itAux;

    itAux = ctrlThreads.find(id);

    if (itAux != ctrlThreads.end())
        return itAux->second->uriRadio;
    else
        return "";
}

vector <int32_t> ThreadPool::getActiveThread()
{
    vector <int32_t> lstRadios;

    for (map<unsigned int, ctrlThread*>::iterator it = ctrlThreads.begin(); it != ctrlThreads.end(); ++it)
        lstRadios.push_back(it->second->idThread);

    return lstRadios;
}
