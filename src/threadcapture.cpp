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
            catch(FifoException& err)
            {
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "Fifo Error: " << *boost::get_error_info<errno_code>(err) << ANSI_COLOR_RESET;
            }
            catch(StreamRadioException& err)
            {
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "StreamRadio Error: " << *boost::get_error_info<errno_code>(err) << ANSI_COLOR_RESET;
            }
            catch(GeneralException& err)
            {
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "Error: " << *boost::get_error_info<errno_code>(err) << ANSI_COLOR_RESET;
            }
            catch(...)
            {
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "Error: General erros" << ANSI_COLOR_RESET;
            }

            if (objRadio->getStatus() != EnumStatusConnect::MIR_CONNECTION_OPEN)
            {
                delete objRadio;
                objRadio = NULL;
                boost::this_thread::sleep(boost::posix_time::seconds(30));
            }
        } while (objRadio == NULL);

        try
        {
/**
            objSlice = new SliceProcess(mrOn, svFP, ipRecognition, portRecognition, sqlConnString,
                                        cutFolder, idThread, Filters, objRadio);
/**/
            objSlice = new SliceProcess();

            objSlice->mrOn = mrOn;
            objSlice->svFP = svFP;
            objSlice->ipRecognition = ipRecognition;
            objSlice->portRecognition = portRecognition;
            objSlice->mySqlConnString = mySqlConnString;
            objSlice->cutFolder = cutFolder;
            objSlice->idRadio = idThread;
            objSlice->Filters = Filters;

            objSlice->objRadio = objRadio;
/**/
            objThreadRadio = new boost::thread(boost::bind(&StreamRadio::read, objRadio));

            do
            {
                boost::this_thread::sleep(boost::posix_time::microseconds(500));
            } while (objRadio->getQueueSize() == 0);

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

    status = 1;
}
