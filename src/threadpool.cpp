#include "threadpool.h"

using namespace std;

ThreadPool::ThreadPool(int mrOn, string ipRecognition, string portRecognition,
                   string sqlConnString, string cutFolder, vector<Filter> *Filters)
{
        this->mrOn = mrOn;
        this->ipRecognition = ipRecognition;
        this->portRecognition = portRecognition;

        this->sqlConnString = sqlConnString;

        this->cutFolder = cutFolder;

        this->Filters = Filters;
}

ThreadPool::~ThreadPool()
{
    //dtor
}

void ThreadPool::addThreads(string uriRadio, int idRadio)
{
    ctrlThread* objThreadControl;

    try
    {
        objThreadControl = new ctrlThread;
        objThreadControl->isStop = true;
        objThreadControl->idThread = idRadio;

        objThreadControl->objCapture = new ThreadCapture(mrOn, ipRecognition, portRecognition,
                                                         sqlConnString, idRadio, uriRadio,
                                                         Filters, cutFolder);
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
        objThreadControl->objCapture->thrClose();
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
