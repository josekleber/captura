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
