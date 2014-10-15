#include "threadcapture.h"

using namespace std;

ThreadCapture::ThreadCapture()
{
    status = CAPTURE_STATUS::OFF;
    stopThread = false;

    objRadio = NULL;
    objSlice = NULL;
}

ThreadCapture::~ThreadCapture()
{
    stopThread = true;

    while (status == CAPTURE_STATUS::ON)
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
        if (objRadio == NULL)
        {
            while (objRadio == NULL)
            {
                bool isError = false;

                try
                {
                    status = CAPTURE_STATUS::STANDBY;
                    objRadio = new StreamRadio;
                    objRadio->open(idThread, uriRadio);
                }
                catch(BadAllocException& err)
                {
                    isError = true;
                    objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
                }
                catch(OpenConnectionException& err)
                {
                    isError = true;
                    objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
                }
                catch(FifoException& err)
                {
                    isError = true;
                    objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
                }
                catch(StreamRadioException& err)
                {
                    isError = true;
                    objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
                }
                catch(GeneralException& err)
                {
                    isError = true;
                    objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
                }
                catch (SignalException& err)
                {
                    isError = true;
                    objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
                }
                catch(...)
                {
                    isError = true;
                    objLog->mr_printf(MR_LOG_ERROR, idThread, "General erros\n");
                }

                if (objRadio)
                {
                    if (isError || (objRadio->getStatus() != EnumStatusConnect::MIR_CONNECTION_OPEN))
                    {
                        delete objRadio;
                        objRadio = NULL;
                        sleep(30);
                    }
                    else
                        status = CAPTURE_STATUS::ON;
                }
            }

            try
            {
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
                }
                catch(...)
                {
                    throw ExceptionClass("threadcapture", "thrRun", "Erro na inicializacao do objeto de processamento de frames");
                }

                try
                {
                    objThreadRadio = new thread(&StreamRadio::read, objRadio);
                    objThreadRadio->detach();
                    while (objRadio->getQueueSize() <= 0)
                        usleep(10);
                    objThreadProcessa = new thread(&SliceProcess::thrProcessa, objSlice);
                    objThreadProcessa->detach();
                }
                catch(...)
                {
                    throw ExceptionClass("threadcapture", "thrRun", "Erro ao iniciar as threads de processamento de captura/frames");
                }
            }
            catch(ExceptionClass& err)
            {
                status = CAPTURE_STATUS::ERROR;
                objLog->mr_printf(MR_LOG_ERROR, idThread, "%s\n", err.what());
            }
        }

        if (objRadio != NULL)
        {
            if ((objRadio->getStatus() != EnumStatusConnect::MIR_CONNECTION_OPEN) || (status != CAPTURE_STATUS::ON))
            {
                if (objSlice != NULL)
                {
                    delete objSlice;
                    objSlice = NULL;
                }

                objRadio->close();
                objRadio = NULL;

                status = CAPTURE_STATUS::OFF;

                break;
            }
        }

        usleep(1);
    }

    status = CAPTURE_STATUS::OFF;
}
