#include "threadpool.h"

using namespace std;

ThreadPool::ThreadPool(int mrOn, bool svFP, string ipRecognition, string portRecognition,
                   string mySqlConnString, string cutFolder, vector<Filter> *Filters)
{
        this->mrOn = mrOn;
        this->svFP = svFP;
        this->ipRecognition = ipRecognition;
        this->portRecognition = portRecognition;

        this->mySqlConnString = mySqlConnString;

        this->cutFolder = cutFolder;

        this->Filters = Filters;
}

ThreadPool::~ThreadPool()
{
    ctrlThreads.clear();
}

void ThreadPool::addThreads(string uriRadio, int idRadio)
{
    ctrlThread* objThreadControl;

    try
    {
        objThreadControl = new ctrlThread;
        objThreadControl->isStop = true;
        objThreadControl->idThread = idRadio;
        objThreadControl->uriRadio = uriRadio;

        objThreadControl->objCapture = new ThreadCapture();
        objThreadControl->objCapture->mrOn = mrOn;
        objThreadControl->objCapture->svFP = svFP;
        objThreadControl->objCapture->ipRecognition = ipRecognition;
        objThreadControl->objCapture->portRecognition = portRecognition;
        objThreadControl->objCapture->mySqlConnString = mySqlConnString;

        objThreadControl->objCapture->idThread = idRadio;
        objThreadControl->objCapture->uriRadio = uriRadio;
        objThreadControl->objCapture->Filters = Filters;
        objThreadControl->objCapture->cutFolder = cutFolder;
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
            ctrlThreads[idRadio] = objThreadControl;
        else
        {
            delete objThreadControl->objCapture;
            delete objThreadControl;
        }
    }
    catch(...)
    {
        throw;
    }
}

void ThreadPool::stopThread(int idRadio)
{
    std::map<unsigned int, ctrlThread*>::iterator itAux;
    ctrlThread* objThreadControl;

    itAux = ctrlThreads.find(idRadio);

    if (itAux != ctrlThreads.end())
    {
        objThreadControl = itAux->second;
        ctrlThreads.erase(itAux);
    }
}

string ThreadPool::getUrlRadio(int idRadio)
{
    std::map<unsigned int, ctrlThread*>::iterator itAux;

    itAux = ctrlThreads.find(idRadio);

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
