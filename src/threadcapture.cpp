#include "threadcapture.h"

using namespace std;

ThreadCapture::ThreadCapture()
{
    status = 0;
    stopThread = false;
}

ThreadCapture::ThreadCapture(int mrOn, bool svFP, string ipRecognition, string portRecognition,
                      string mySqlConnString, int idThread, string uriRadio,
                      vector<Filter> *Filters, string cutFolder)
{
    this->mrOn = mrOn;
    this->svFP = svFP;
    this->ipRecognition = ipRecognition;
    this->portRecognition = portRecognition;
    this->mySqlConnString = mySqlConnString;

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
        usleep(10);

    if (objRadio != NULL)
    {
        delete objRadio;
    }

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
                objRadio->open(idThread, uriRadio);
            }
            catch(BadAllocException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(OpenConnectionException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(FifoException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(StreamRadioException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(GeneralException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(...)
            {
                objLog->mr_printf(MR_LOG_ERROR, idThread, "General erros\n");
            }

            if (objRadio->getStatus() != EnumStatusConnect::MIR_CONNECTION_OPEN)
            {
                delete objRadio;
                objRadio = NULL;
                sleep(30);
            }
        } while (objRadio == NULL);

        try
        {
            objSlice = new SliceProcess();

            objSlice->mrOn = mrOn;
            objSlice->svFP = svFP;
            objSlice->ipRecognition = ipRecognition;
            objSlice->portRecognition = portRecognition;
            objSlice->mySqlConnString = mySqlConnString;
            objSlice->cutFolder = cutFolder;
            objSlice->idRadio = idThread;
            objSlice->Filters = Filters;
            objSlice->MutexAccess = MutexAccess;

            objSlice->objRadio = objRadio;
            objThreadRadio = new thread(&StreamRadio::read, objRadio);

            do
            {
                usleep(100);
            } while (objRadio->getQueueSize() == 0);

            objThreadProcessa = new thread(&SliceProcess::thrProcessa, objSlice);

            while (!stopThread)
            {
                if (objRadio->getStatus() != EnumStatusConnect::MIR_CONNECTION_OPEN)
                {
                    delete objSlice;
                    objSlice = NULL;

                    objRadio->close();
                    objRadio = NULL;

                    break;
                }
                usleep(1);
            }
        }
        catch(...)
        {
            status = -1;
            throw;
        }
    }

    status = 1;
}
