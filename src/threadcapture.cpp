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
    // conectar no sql server

    // conectar na radio

    try
    {
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
