#include "rawdata.h"

RAWData::RAWData() : Parser ()
{
    this->audioFormat = RAWData::raw;

    this->setBitRate(64000);
    this->setChannels(1);
    this->setSampleRate(11025);

    this->freq = RAW_SAMPLE_RATE;
}

RAWData::~RAWData()
{
    //dtor
}

unsigned int* RAWData::CreateFingerPrint(vector <uint8_t> Data, unsigned int* FingerPrintSize, bool mltFFT)
{
    unsigned int len = Data.size();

    uint8_t* convArray = new uint8_t[len];
    for (unsigned int i = 0; i < len; i++)
        convArray[i] = Data[i] & 0xff;

    try
    {
        unsigned int *bits;
        FingerPrint* objFingerPrint = new FingerPrint(Filters, (short int*)convArray, len / 2,
                cdc_ctx_out->sample_rate, mltFFT, MutexAccess,
                &bits, FingerPrintSize);
        objFingerPrint->Generate();

        delete[] convArray;
        delete objFingerPrint;

        return bits;
    }
    catch(FingerPrintException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "FingerPrint object create error : %d\n", (*boost::get_error_info<errmsg_info>(err)).c_str());

        return NULL;
    }
}

void RAWData::Execute()
{
clock_t start;
start = clock();
    try
    {
        Resample();
    }
    catch(ResampleException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (FP): %d\n", *boost::get_error_info<errno_code>(err));
        return;
    }
    catch(FifoException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (FP): %d\n", *boost::get_error_info<errno_code>(err));
        return;
    }
    catch(BadAllocException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (FP): %d\n", *boost::get_error_info<errno_code>(err));
        return;
    }
    catch(FFMpegException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (FP): %d\n", *boost::get_error_info<errno_code>(err));
        return;
    }

    // gerando fingerprints
    unsigned int nbits;
    unsigned int* bits = NULL;
    bits = CreateFingerPrint(binOutput, &nbits, true);

    binOutput.clear();

    if (bits != NULL)
    {
        int idMySql = 0;
        bool okMySql = true;
/**
        try
        {
            string aux = fileName.substr(0, fileName.find_last_of(".")) + "mp3";
            objMySql = new Database_MySql(mySqlConnString);

            objMySql->open();
            idMySql = objMySql->insertRecorte(idRadio, nbits, aux, bits);
            objMySql->close();
        }
        catch(...)
        {
            okMySql = false;

            objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro ao registrar no banco de dados para contingencia");
        }
/**/

        try
        {
            if (svFP || !okMySql)
            {
                int p = fileName.find_last_of("/") + 1;
                string aux = fileName.substr(0, p) + "FingerPrint";
                boost::filesystem::path dirPath(aux);

                if (!boost::filesystem::exists(dirPath))
                {
                    boost::filesystem::create_directories(dirPath);
                }
                aux += "/" + fileName.substr(p , fileName.find_last_of(".") - p) + ".bin";

                FILE* fp = fopen(aux.c_str(), "wb");
                fwrite((uint8_t*)&bits[0], nbits, 4, fp);
                fclose(fp);
            }
        }
        catch(...)
        {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro na gravacao do FingerPrint em arquivo\n");
        }

        // enviando dados para o mrserver
        if ((mrOn != 0) && okMySql)
        {
            // preparando vetor com dados para envio via socket
            uint8_t* conv;
            int pos = 0;

            int16_t arqNameSize = fileName.size();
            uint8_t* buff  = new uint8_t[4 * nbits + arqNameSize + 14];

            conv = (uint8_t*)&this->idRadio;
            for (int i = 0; i < 4; buff[pos++] = conv[i++]);

            conv = (uint8_t*)&idMySql;
            for (int i = 0; i < 4; buff[pos++] = conv[i++]);

            conv = (uint8_t*)&nbits;
            for (int i = 0; i < 4; buff[pos++] = conv[i++]);

            conv = (uint8_t*)&arqNameSize;
            for (int i = 0; i < 2; buff[pos++] = conv[i++]);

            for (int i = 0; i < arqNameSize - 3; buff[pos++] = (fileName.c_str())[i++]);
            buff[pos++] = 'm';
            buff[pos++] = 'p';
            buff[pos++] = '3';

            for (unsigned int j = 0; j < nbits; j++)
            {
                conv = (uint8_t*)&bits[j];
                for (int i = 0; i < 4; buff[pos++] = conv[i++]);
            }

            delete[] bits;

            try
            {
                boost::asio::io_service IO_Service;
                tcp::resolver Resolver(IO_Service);
                tcp::resolver::query Query(ipRecognition, portRecognition);
                tcp::resolver::iterator EndPointIterator = Resolver.resolve(Query);

                TCPClient* objClient = new TCPClient(IO_Service, EndPointIterator, buff, pos);

                boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &IO_Service));

                int cntTimeOut = 0;
                while ((objClient->strResp == "") && (cntTimeOut < SOCKET_TIMEOUT / 10))
                {
//                    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
                    usleep(10);
                    cntTimeOut++;
                }

                if (cntTimeOut >= SOCKET_TIMEOUT / 10)
                    BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "TimeOut no socket : " << objClient->strResp << ANSI_COLOR_RESET;
                else if (objClient->strResp != "Received")
                    BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "Erro de envio : " << objClient->strResp << ANSI_COLOR_RESET;

                objClient->Close();
            }
            catch(ConvertException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "%s\n", err.what());
            }
            catch (exception& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "%s\n", err.what());
            }

            delete[] buff;
        }

objLog->mr_printf(MR_LOG_DEBUG, idRadio, "MySql : %d    Recorte : %d    Tempo de processamento : %8.4f\n", idMySql, idSlice, (float)(clock() - start)/CLOCKS_PER_SEC);
start = clock();
    }

    delete this;
}

void RAWData::EndResample()
{
    try
    {
        for (int i = 0; i < pkt_out.size; i++)
            binOutput.push_back(pkt_out.data[i]);
    }
    catch(...)
    {
        throw BadAllocException() << errno_code(MIR_ERR_RAW_VECTOR_ALLOC);
    }
}
