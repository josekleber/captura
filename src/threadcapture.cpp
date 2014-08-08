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

void ThreadCapture::thrRun()
{
    try
    {
        boost::thread* objThreadRadio;
        boost::thread* objThreadRawParser;
        boost::thread* objThreadM4aParser;

        Parser* objParser = new Parser;
        objParser->Filters = Filters;
        objParser->ipRecognition = ipRecognition;
        objParser->portRecognition = portRecognition;
        objParser->sqlConnString = sqlConnString;
        objParser->cutFolder = cutFolder;

        StreamRadio* objRadio = new StreamRadio;

        objRadio->open(uriRadio);

        objParser->SetStreamRadio(idThread, objRadio);
        objParser->CreateContext("eu.wav", true, NULL);

        objThreadRadio = new boost::thread(boost::bind(&StreamRadio::read, objRadio));
        sleep(2);
        while (objRadio->getFifoSize() == 0)
        {
            cout << objRadio->getFifoSize() << endl;
            sleep(2);
        };
        objThreadRawParser = new boost::thread(boost::bind(&Parser::ProcessFrames, objParser));
        objThreadM4aParser = new boost::thread(boost::bind(&Parser::ProcessOutput, objParser));

        while (!stopThread);
/**
        while (!stopThread)
        {
            printf("%s\n", uriRadio.c_str());
            for (int i = 0; i < 2000000; i++);

            // coletar recorte

            // criar fingerprint

            // mudar status sql

            // consultar na base

            // mudar status sql
        }
/**/
    }
    catch(...)
    {
        throw;
    }
}

void ThreadCapture::thrClose()
{
    stopThread = true;
}
