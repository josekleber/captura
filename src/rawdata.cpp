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
    int idMySql = 0;

clock_t start = clock();

    if (idRadio == 0)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Radio sem id\n");
        return;
    }

    try
    {
        Resample();
    }
    catch(SignalException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (FP): %s\n", err.what());
        return;
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
    try
    {
        if (binOutput.size() > 0)
            bits = CreateFingerPrint(binOutput, &nbits, true);

        binOutput.clear();
    }
    catch(SignalException& err)
    {
        binOutput.clear();
        if (bits != NULL)
            delete[] bits;
        bits = NULL;

        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro na criacao dos FingerPrints\n");
    }

    if (bits != NULL)
    {
        bool okMySql = true;
/**  Rotina para o MySQL
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
        catch(ExceptionClass& err)
        {
            objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro de segmentacao na rotina de gravacao do FingerPrint\n");
        }
        catch(...)
        {
            objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro na gravacao do FingerPrint em arquivo\n");
        }

        // enviando dados para o mrserver
        if ((mrOn != 0) && okMySql)
        {
            try
            {
                // preparando vetor com dados para envio via socket
                uint8_t* conv;
                uint8_t* buff;
                int pos = 0;

                int16_t arqNameSize = fileName.size();

                try
                {
                    buff  = new uint8_t[4 * nbits + arqNameSize + 14];
                }
                catch(...)
                {
                    throw ExceptionClass("rawdata", "Execute", "Erro na alocacao do vetor de dados para transmissao");
                }

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
                        usleep(10);
                        cntTimeOut++;
                    }

                    if (cntTimeOut >= SOCKET_TIMEOUT / 10)
                        objLog->mr_printf(MR_LOG_ERROR, idRadio, "TimeOut no socket : %s\n", objClient->strResp.c_str());
                    else if (objClient->strResp != "Received")
                        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro de envio : %s\n", objClient->strResp.c_str());

                    objClient->Close();
                }
                catch(SignalException& err)
                {
                    throw;
                }
                catch (exception& err)
                {
                    throw;
                }
                catch (...)
                {
                    throw ExceptionClass("rawdata", "Execute", "Erro geral na class RAWData, Execute");
                }

                delete[] buff;
            }
            catch(SignalException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro de Segmentacao na rotina Execute do rawdata\n", err.what());
            }
            catch(ExceptionClass& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "%s\n", err.what());
            }
            catch (exception& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "%s\n", err.what());
            }
            catch (...)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "rawdata (Execute) : Erro Desconhecido\n");
            }
        }
/**/

        delete[] bits;
    }
objLog->mr_printf(MR_LOG_DEBUG, idRadio, "RawData >>> MySql : %5d    Recorte : %5d    Tempo de processamento : %8.4f    Fifo : %d\n", idMySql, idSlice, (float)(clock() - start)/CLOCKS_PER_SEC, szFifo);
}

void RAWData::EndResample()
{
    try
    {
        for (int i = 0; i < pkt_out.size; i++)
            binOutput.push_back(pkt_out.data[i]);
    }
    catch(SignalException& err)
    {
        throw ExceptionClass("rawdata", "EndResample", "Erro de Segmentacao");
    }
    catch(...)
    {
        throw BadAllocException() << errno_code(MIR_ERR_RAW_VECTOR_ALLOC);
    }
}
