#include "threadcapture.h"

using namespace std;

ThreadCapture::ThreadCapture()
{
    status = 0;
    stopThread = false;
}

ThreadCapture::~ThreadCapture()
{
    stopThread = true;
}

void ThreadCapture::thrRun()
{
    try
    {
        objRadio = new StreamRadio;
        objRadio->open(uriRadio);

        objSlice = new SliceProcess(ipRecognition, portRecognition, sqlConnString,
                                    cutFolder, idThread, Filters, objRadio);

        objThreadRadio = new boost::thread(boost::bind(&StreamRadio::read, objRadio));
        boost::this_thread::sleep(boost::posix_time::microseconds(500));
        while (objRadio->getFifoSize() == 0)
        {
            boost::this_thread::sleep(boost::posix_time::microseconds(500));
        }

        objThreadProcessa = new boost::thread(boost::bind(&SliceProcess::thrProcessa, objSlice));

        while (!stopThread)
        {
            boost::this_thread::sleep(boost::posix_time::microseconds(1));
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
