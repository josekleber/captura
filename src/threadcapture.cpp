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
        try
        {
            // abrindo uma radio
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
                    catch (SignalException& err)
                    {
                        isError = true;
                        objLog->mr_printf(MR_LOG_ERROR, idThread, "%d\n", *boost::get_error_info<errno_code>(err));
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

                            continue;
                        }

                        try
                        {
                            objThreadRadio = new thread(&StreamRadio::read, objRadio);
                            objThreadRadio->detach();
                        }
                        catch(SignalException& err)
                        {
                            throw ExceptionClass("threadcapture", "thrRun", "Erro de segmentacao na criacao do objRadio");
                        }
                        catch(...)
                        {
                            throw ExceptionClass("threadcapture", "thrRun", "Erro ao iniciar a thread de captura");
                        }
                    }
                }
            }

            // criando objetp SliceProcess
            if (objSlice == NULL)
            {
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
                    }
                    catch(SignalException& err)
                    {
                        throw ExceptionClass("threadcapture", "thrRun", "Erro de segmentacao na criacao do objSlice");
                    }
                    catch(...)
                    {
                        throw ExceptionClass("threadcapture", "thrRun", "Erro na inicializacao do objeto de processamento de frames");
                    }

                    try
                    {
                        objSlice->objRadio = objRadio;

                        objThreadProcessa = new thread(&SliceProcess::thrProcessa, objSlice);
                        objThreadProcessa->detach();

                        sleep(1);
                    }
                    catch(SignalException& err)
                    {
                        throw ExceptionClass("threadcapture", "thrRun", "Erro de segmentacao no thread do objSlice");
                    }
                    catch(...)
                    {
                        throw ExceptionClass("threadcapture", "thrRun", "Erro ao iniciar a thread de processamento de frames");
                    }
                }
                catch(SignalException& err)
                {
                    status = CAPTURE_STATUS::STANDBY;
                    throw ExceptionClass("threadcapture", "thrRun", "Erro de segmentacao geral");
                }
                catch(ExceptionClass& err)
                {
                    status = CAPTURE_STATUS::STANDBY;
                    throw;
                }
            }

            if (objRadio != NULL)
            {
                // verificando se conexao com a radio esta estavel
                if (objRadio->getStatus() != EnumStatusConnect::MIR_CONNECTION_OPEN)
                {
                    if (objSlice != NULL)
                    {
                        delete objSlice;
                        objSlice = NULL;
                    }

                    objRadio->close();
                    objRadio = NULL;
                }
            }

            if (objSlice != NULL)
            {
                if (objSlice->getStatus() != SliceProcess::RUN)
                {
                    delete objSlice;
                    objSlice = NULL;
                }
            }

            if (objRadio == NULL)
                status = CAPTURE_STATUS::ERROR;
            else if (objSlice == NULL)
                status = CAPTURE_STATUS::STANDBY;
            else if ((objRadio->getStatus() == EnumStatusConnect::MIR_CONNECTION_OPEN) && (objSlice->getStatus() != SliceProcess::RUN))
                status = CAPTURE_STATUS::STANDBY;
            else if ((objRadio->getStatus() == EnumStatusConnect::MIR_CONNECTION_OPEN) && (objSlice->getStatus() == SliceProcess::RUN))
                status = CAPTURE_STATUS::ON;
        }
        catch(SignalException& err)
        {
            objLog->mr_printf(MR_LOG_ERROR, idThread, "Erro de SignalException (%s)\n", err.what());
        }
        catch(ExceptionClass& err)
        {
            objLog->mr_printf(MR_LOG_ERROR, idThread, "%s\n", err.what());
        }
        catch(...)
        {
            objLog->mr_printf(MR_LOG_ERROR, idThread, "threadcapture (thrRun) : Erro desconhecido");
        }

        usleep(1);
    }

    status = CAPTURE_STATUS::OFF;
}
