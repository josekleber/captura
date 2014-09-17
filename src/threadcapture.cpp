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
}

void ThreadCapture::thrRun()
{
    while (!stopThread)
    {
        do
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
                objRadio->close();
                delete objRadio;
                objRadio = NULL;
                boost::this_thread::sleep(boost::posix_time::seconds(30));
            }
        } while (objRadio == NULL);

        try
        {
            objSlice = new SliceProcess(mrOn, ipRecognition, portRecognition, sqlConnString,
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
    }
}

void ThreadCapture::thrClose()
{
    stopThread = true;
}
