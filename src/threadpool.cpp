#include "threadpool.h"

using namespace std;

ThreadPool::ThreadPool(int mrOn, bool svFP, string ipRecognition, string portRecognition, string ipResult, string portResult,
                   string mySqlConnString, string cutFolder, vector<Filter> *Filters)
{
    this->mrOn = mrOn;
    this->svFP = svFP;
    this->ipRecognition = ipRecognition;
    this->portRecognition = portRecognition;

    this->ipResult = ipResult;
    this->portResult = portResult;

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
    if ((idRadio <= 0) || (uriRadio == ""))
        throw ExceptionClass("threadpool", "addThreads", "id ou url errado");

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
        objThreadControl->objCapture->ipResult = ipResult;
        objThreadControl->objCapture->portResult = portResult;
        objThreadControl->objCapture->mySqlConnString = mySqlConnString;

        objThreadControl->objCapture->idThread = idRadio;
        objThreadControl->objCapture->uriRadio = uriRadio;
        objThreadControl->objCapture->Filters = Filters;
        objThreadControl->objCapture->cutFolder = cutFolder;

        objThreadControl->objCapture->MutexAccess = &MutexAccess;
    }
    catch(SignalException& err)
    {
        throw;
    }
    catch(...)
    {
        if (objThreadControl->objCapture)
            delete objThreadControl->objCapture;

        throw;
    }

    try
    {
        objThreadControl->objThread = new thread(&ThreadCapture::thrRun, std::ref(objThreadControl->objCapture));
        objThreadControl->objThread->detach();
        objThreadControl->isStop = false;

        ctrlThreads[idRadio] = objThreadControl;
    }
    catch (SignalException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro : %s\n", err.what());
    }
    catch(...)
    {
        throw;
    }
}

void ThreadPool::stopThread(int idRadio)
{
    std::map<unsigned int, ctrlThread*>::iterator itAux;

    itAux = ctrlThreads.find(idRadio);

    if (itAux != ctrlThreads.end())
    {
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
