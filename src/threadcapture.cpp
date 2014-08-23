#include "threadcapture.h"

using namespace std;

ThreadCapture::ThreadCapture()
{
    status = 0;
    stopThread = false;
}

ThreadCapture::~ThreadCapture()
{
    stopThread = false;
}

int ThreadCapture::init()
{
    try
    {
        objParser = new Parser;
        objParser->Filters = Filters;
        objParser->ipRecognition = ipRecognition;
        objParser->portRecognition = portRecognition;
        objParser->sqlConnString = sqlConnString;
        objParser->cutFolder = cutFolder;
    }
    catch(...)
    {
        status = -1;
        throw;
    }

    try
    {
        objRadio = new StreamRadio;
        objRadio->open(uriRadio);
    }
    catch(...)
    {
        status = -1;
        throw;
    }

    try
    {
        objParser->SetStreamRadio(idThread, objRadio);
        objParser->CreateContext("eu.wav", true, NULL);
    }
    catch(...)
    {
        status = -1;
        throw;
    }
}

void ThreadCapture::thrRun()
{
    try
    {
        objThreadRadio = new boost::thread(boost::bind(&StreamRadio::read, objRadio));
        sleep(2);
        while (objRadio->getFifoSize() == 0)
        {
            cout << objRadio->getFifoSize() << endl;
            sleep(2);
        };
        objThreadRawParser = new boost::thread(boost::bind(&Parser::ProcessFrames, objParser));
        objThreadM4aParser = new boost::thread(boost::bind(&Parser::ProcessOutput, objParser));

        while (!stopThread)
        {
            sleep(1);
        }
    }
    catch(...)
    {
        status = -1;
        throw;
    }
}

void ThreadCapture::thrClose()
{
    stopThread = true;
}
