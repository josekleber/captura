#include "sliceprocess.h"

SliceProcess::SliceProcess()
{
    stopThread = false;
    this->Status = enumSliceProcess::STOP;
}

SliceProcess::~SliceProcess()
{
    stopThread = true;

    boost::this_thread::sleep(boost::posix_time::seconds(10));

    for (int i = 0; i < (int)aux.size(); i++)
        if (aux[i].size() > 0)
            aux[i].clear();
    if (aux.size() > 0)
        aux.clear();

    for (int i = 0; i < (int)Packets.size(); i++)
    {
        for (int j = 0; j < (int)Packets[i].size(); j++)
            if (Packets[i][j].size() > 0)
                Packets[i][j].clear();
        if (Packets[i].size() > 0)
            Packets[i].clear();
    }
    if (Packets.size() > 0)
        Packets.clear();
}


void SliceProcess::thrProcessa()
{
    this->Status = enumSliceProcess::STOP;

    try
    {
        int szFifo;
        int szFrame;

        objRawData = NULL;
        objFileData = NULL;

        idSlice = 0;

        try
        {
            cdc_ctx_in = objRadio->getCodecContext();

        }
        catch(...)
        {
            throw ExceptionClass("sliceprocess", "thrProcessa", "erro ao tentar recuperar o codec context do objRadio.");
        }

        int qtdPack = 0;

        // criando objeto para fingerprint
        try
        {
            objRawData = new RAWData();
            objRawData->idRadio = idRadio;
            objRawData->cdc_ctx_in = cdc_ctx_in;
            objRawData->bitRateIn = cdc_ctx_in->bit_rate;
            objRawData->nbChannelsIn = cdc_ctx_in->channels;
            objRawData->nbSamplesIn = cdc_ctx_in->frame_size;
            objRawData->sampleFormatIn = objRadio->getFrameFormat();
            objRawData->sampleRateIn = cdc_ctx_in->sample_rate;
            objRawData->channelLayoutIn = objRadio->getChannelLayout();
            objRawData->fileName = "SkySoft.wav";

            objRawData->Filters = Filters;
            objRawData->mrOn = mrOn;
            objRawData->svFP = svFP;
            objRawData->MutexAccess = MutexAccess;
            objRawData->ipRecognition = ipRecognition;
            objRawData->portRecognition = portRecognition;
            objRawData->mySqlConnString = mySqlConnString;

            objRawData->Config();
        }
        catch(SignalException& err)
        {
            throw ExceptionClass("sliceprocess", "thrProcessa", "Erro de Segmentacao na cricao do objRawData");
        }
        catch(...)
        {
            throw RawDataException() << errno_code(MIR_ERR_RAWDATA_GENERATION);
        }

        // criando objeto para arquivo
        try
        {
            objFileData = new FileData();
            objFileData->idRadio = idRadio;
            objFileData->cdc_ctx_in = cdc_ctx_in;
            objFileData->bitRateIn = cdc_ctx_in->bit_rate;
            objFileData->nbChannelsIn = cdc_ctx_in->channels;
            objFileData->nbSamplesIn = cdc_ctx_in->frame_size;
            objFileData->sampleFormatIn = objRadio->getFrameFormat();
            objFileData->sampleRateIn = cdc_ctx_in->sample_rate;
            objFileData->channelLayoutIn = objRadio->getChannelLayout();
            objFileData->fileName = "SkySoft.mp3";

            objFileData->Config();
        }
        catch(SignalException& err)
        {
            throw ExceptionClass("sliceprocess", "thrProcessa", "Erro de Segmentacao na cricao do objFileData");
        }
        catch(...)
        {
            throw FileDataException() << errno_code(MIR_ERR_FILEDATA_GENERATION);
        }

        this->Status = enumSliceProcess::RUN;

        while (!stopThread)
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(30));
            try
            {
                szFrame = objRadio->getFrameSize();
                szFifo = objRadio->getQueueSize();

                if (szFifo >= szFrame)
                {
                    // carregando dados da FIFO
/**
objRadio->delQueueData();
boost::this_thread::sleep(boost::posix_time::seconds(1));
continue;
/**/
                    aux = objRadio->getQueueData();
                    if (aux.size() > 0)
                    {
                        Packets.push_back(aux);
                        for (int i = 0; i < (int)aux.size(); i++)
                            if (aux[i].size() > 0)
                                aux[i].clear();
                        aux.clear();

                        qtdPack += szFrame;
                    }

                    if (qtdPack >= (objRadio->getFrameSampleRate()  * 5.2))
                    {
                        qtdPack = 0;
                        char chrArqName[28];
                        sprintf(chrArqName, "%05d-%05d-%s", idRadio, idSlice, getDateTime().c_str());
                        string arqName = chrArqName;
                        try
                        {
                            arqName = getSaveCutDir() + "/" + arqName;
                        }
                        catch(SignalException& err)
                        {
                            throw ExceptionClass("sliceprocess", "thrProcessa", "Erro de Segmentacao");
                        }
                        catch(exception& err)
                        {
                            throw;
                        }

/**/
                        // rodando a thread do fingerprint
                        try
                        {
                            objRawData->setBuffer(arqName + ".wav", Packets, szFrame, objRadio->getFrameFormat(),
                                                  objRadio->getFrameSampleRate(), objRadio->getFrameChannelLayout(),
                                                  objRadio->getFrameChannels());
                            objRawData->idSlice = idSlice;
                            objRawData->szFifo = objRadio->getQueueSize();
                            objThreadRawParser = new thread(&RAWData::Execute, objRawData);
                            objThreadRawParser->detach();
                        }
                        catch(SignalException& err)
                        {
                            throw ExceptionClass("sliceprocess", "thrProcessa", "Erro de Segmentacao no thread do objRawData");
                        }
                        catch(...)
                        {
                            throw;
                        }

/**/
                        // rodando a thread da gravacao do arquivo mp3
                        try
                        {
                            objFileData->setBuffer(arqName + ".mp3", Packets, szFrame, objRadio->getFrameFormat(),
                                                  objRadio->getFrameSampleRate(), objRadio->getFrameChannelLayout(),
                                                  objRadio->getFrameChannels());
                            objThreadArqParser = new thread(&FileData::Execute, objFileData);
                            objThreadArqParser->detach();
                        }
                        catch(SignalException& err)
                        {
                            throw ExceptionClass("sliceprocess", "thrProcessa", "Erro de Segmentacao no thread do objFileData");
                        }
                        catch(...)
                        {
                            throw;
                        }
/**/

                        // reseta dados
                        try
                        {
                            for (int i = 0; i < (int)Packets.size(); i++)
                            {
                                for (int j = 0; j < (int)Packets[i].size(); j++)
                                    if (Packets[i][j].size() > 0)
                                        Packets[i][j].clear();
                                if (Packets[i].size() > 0)
                                    Packets[i].clear();
                            }
                            if (Packets.size() > 0)
                                Packets.clear();
                        }
                        catch(SignalException& err)
                        {
                            throw ExceptionClass("sliceprocess", "thrProcessa", "Erro de Segmentacao na limpeza dos Packets");
                        }
                        catch(...)
                        {
                            throw;
                        }

//objLog->mr_printf(MR_LOG_DEBUG, idRadio, "Recorte : %5d    Fifo : %d\n", idSlice, objRadio->getQueueSize());
                        idSlice++;

                        boost::this_thread::sleep(boost::posix_time::seconds(1));
                    }
                }
            }
            catch(SignalException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro de segmentacao\n");
            }
            catch(ExceptionClass& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "%s\n", err.what());
            }
            catch(FifoException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : FIFO %d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(ConvertException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro na abertura de arquivo : %d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(StreamException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro na abertura de arquivo : %d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(exception& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro desconhecido : %d\n", err.what());
            }
            catch(...)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : General Error\n");
            }
        }
    }
    catch(SignalException& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro de segmentacao - loop principal\n");
    }
    catch(BadAllocException& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro de alocacao de memoria : %d\n", *boost::get_error_info<errno_code>(err));
    }
    catch(RawDataException& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro na criacao do objeto RawData : %d\n", *boost::get_error_info<errno_code>(err));
    }
    catch(FileDataException& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro na criacao do objeto FileData : %d\n", *boost::get_error_info<errno_code>(err));
    }
    catch (ExceptionClass& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, err.what());
    }
    catch(...)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : >>>>>>>>>>>>>>>>>>> Erro grave, sem indice. <<<<<<<<<<<<<<<<<<<\n");
    }

    Status = enumSliceProcess::STOP;
}

string SliceProcess::getDate()
{
    struct tm *DtHr;
    time_t t;
    char strAux[20];

    t = time(NULL);
    DtHr = localtime(&t);

    sprintf(strAux, "%02d%02d%02d", DtHr->tm_year - 100, DtHr->tm_mon + 1, DtHr->tm_mday);

    return strAux;
}

string SliceProcess::getHour()
{
    struct tm *DtHr;
    time_t t;
    char strAux[20];

    t = time(NULL);
    DtHr = localtime(&t);

    sprintf(strAux, "%02d", DtHr->tm_hour);

    return strAux;
}

string SliceProcess::getDateTime()
{
    struct tm *DtHr;
    time_t t;
    char strAux[20];

    t = time(NULL);
    DtHr = localtime(&t);

    sprintf(strAux, "%02d%02d%02d_%02d%02d%02d", DtHr->tm_year - 100, DtHr->tm_mon + 1, DtHr->tm_mday, DtHr->tm_hour, DtHr->tm_min, DtHr->tm_sec);

    return strAux;
}

string SliceProcess::getSaveCutDir()
{
    char strAux[10];
    string path;

    try
    {
        path = this->cutFolder + "/" + getDate();
        fs::path dirPath(path);

        if (!fs::exists(dirPath))
        {
            idSlice = 0;
            fs::create_directories(dirPath);
        }

        path += "/" + getHour();

        fs::path dirPathTime(path);

        if (!fs::exists(dirPathTime))
        {
            fs::create_directories(dirPathTime);
        }

        sprintf(strAux, "%05d", idRadio);

        path += "/" + string(strAux);

        fs::path dirPathRadio(path);

        if (!fs::exists(dirPathRadio))
        {
            fs::create_directories(dirPathRadio);
        }
    }
    catch(exception& err)
    {
        throw OpenFileException() << errno_code(MIR_ERR_CREATE_PATH);
    }

    return path;
}

int SliceProcess::getStatus()
{
    return Status;
}
