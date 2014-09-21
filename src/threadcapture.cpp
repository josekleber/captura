#include "threadcapture.h"

using namespace std;

ThreadCapture::ThreadCapture(int mrOn, string ipRecognition, string portRecognition,
                      string sqlConnString, int idThread, string uriRadio,
                      vector<Filter> *Filters, string cutFolder)
{
    this->mrOn = mrOn;
    this->ipRecognition = ipRecognition;
    this->portRecognition = portRecognition;
    this->sqlConnString = sqlConnString;

    this->idThread = idThread;   // igual a idRadio
    this->uriRadio = uriRadio;
    this->Filters = Filters;
    this->cutFolder = cutFolder;


    status = 0;
    stopThread = false;
}

ThreadCapture::~ThreadCapture()
{
    stopThread = true;

    while (status == 0)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));

    if (objRadio != NULL)
    {
        delete objRadio;
    }

}

void ThreadCapture::thrRun()
{
    while (!stopThread)
    {
        while (objRadio == NULL)
        {
            try
            {
                objRadio = new StreamRadio;
                objRadio->open(uriRadio);
            }
            catch(BadAllocException& err)
            {
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "Error: " << *boost::get_error_info<errno_code>(err) << ANSI_COLOR_RESET;
            }
            catch(OpenConnectionException& err)
            {
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "Error: " << *boost::get_error_info<errno_code>(err) << ANSI_COLOR_RESET;
            }

            if (objRadio->getStatus() != EnumStatusConnect::MIR_CONNECTION_OPEN)
            {
                delete objRadio;
                objRadio = NULL;
                boost::this_thread::sleep(boost::posix_time::seconds(30));
            }
        }
/**/
        try
        {
            objSlice = new SliceProcess(mrOn, ipRecognition, portRecognition, sqlConnString,
                                        cutFolder, idThread, Filters, objRadio);
/**
printf("preparando para iniciar\n");
boost::this_thread::sleep(boost::posix_time::seconds(10));
printf("mais 10 segundos\n");
boost::this_thread::sleep(boost::posix_time::seconds(10));
/**/
            objThreadRadio = new boost::thread(boost::bind(&StreamRadio::read, objRadio));
            boost::this_thread::sleep(boost::posix_time::microseconds(500));
            while (objRadio->getQueueSize() == 0)
            {
                boost::this_thread::sleep(boost::posix_time::microseconds(500));
            }

            objThreadProcessa = new boost::thread(boost::bind(&SliceProcess::thrProcessa, objSlice));

            while (!stopThread)
            {
                if (objRadio->getStatus() != EnumStatusConnect::MIR_CONNECTION_OPEN)
                {
                    delete objSlice;
                    objSlice = NULL;

                    objRadio->close();
                    delete objRadio;
                    objRadio = NULL;

                    break;
                }
                boost::this_thread::sleep(boost::posix_time::microseconds(1));
            }
        }
        catch(...)
        {
            status = -1;
            throw;
        }
/**/
    }

    status = 1;
}
