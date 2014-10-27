#include "sliceprocess.h"

SliceProcess::SliceProcess()
{
    stopThread = false;
    this->Status = enumSliceProcess::STOP;
}

SliceProcess::~SliceProcess()
{
    stopThread = true;

    sleep(10);

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

    av_frame_free(&inFrame);
}


void SliceProcess::thrProcessa()
{
    this->Status = enumSliceProcess::STOP;

    try
    {
        int szFifo;
        int inFrameSize;

        objRawData = NULL;
        objFileData = NULL;

        idSlice = 0;

        inFrame = av_frame_alloc();
        if (inFrame == NULL)
            throw BadAllocException() << errno_code(MIR_ERR_FRAME_ALLOC);

        try
        {
            cdc_ctx_in = objRadio->getCodecContext();

            inFrameSize             = cdc_ctx_in->frame_size;
            inFrame->channel_layout = cdc_ctx_in->channel_layout;
            inFrame->format         = cdc_ctx_in->sample_fmt;
            inFrame->sample_rate    = cdc_ctx_in->sample_rate;
        }
        catch(...)
        {
            throw ExceptionClass("sliceprocess", "thrProcessa", "erro ao tentar recuperar o codec context do objRadio.");
        }

        inFrame->nb_samples = inFrameSize;

        if (av_frame_get_buffer(inFrame, 0) < 0)
            throw BadAllocException() << errno_code(MIR_ERR_FRAME_ALLOC);

        int qtdPack = 0;

        // criando objeto para fingerprint
        try
        {
            objRawData = new RAWData();
            objRawData->idRadio = idRadio;
            objRawData->bitRateIn = cdc_ctx_in->bit_rate;
            objRawData->nbChannelIn = cdc_ctx_in->channels;
            objRawData->nbSamplesIn = inFrameSize;
            objRawData->sampleFormatIn = cdc_ctx_in->sample_fmt;
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
            av_frame_free(&inFrame);
            throw RawDataException() << errno_code(MIR_ERR_RAWDATA_GENERATION);
        }

        // criando objeto para arquivo
        try
        {
            objFileData = new FileData();
            objFileData->idRadio = idRadio;
            objFileData->bitRateIn = cdc_ctx_in->bit_rate;
            objFileData->nbChannelIn = cdc_ctx_in->channels;
            objFileData->nbSamplesIn = inFrameSize;
            objFileData->sampleFormatIn = cdc_ctx_in->sample_fmt;
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
            av_frame_free(&inFrame);
            throw FileDataException() << errno_code(MIR_ERR_FILEDATA_GENERATION);
        }

        this->Status = enumSliceProcess::RUN;

        while (!stopThread)
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(30));
            try
            {
                szFifo = objRadio->getQueueSize();

                if (szFifo >= inFrameSize)
                {
                    // carregando dados da FIFO
//objRadio->delQueueData();
                    aux = objRadio->getQueueData();
                    if (aux.size() > 0)
                    {
                        Packets.push_back(aux);
                        for (int i = 0; i < (int)aux.size(); i++)
                            if (aux[i].size() > 0)
                                aux[i].clear();
                        aux.clear();

                        qtdPack += inFrameSize;
                    }

                    if (qtdPack >= (inFrame->sample_rate  * 5.2))
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

                        // atualizando dados do codec context
                        try
                        {
                            cdc_ctx_in = objRadio->getCodecContext();

                            inFrameSize             = cdc_ctx_in->frame_size;
                            inFrame->channel_layout = cdc_ctx_in->channel_layout;
                            inFrame->format         = cdc_ctx_in->sample_fmt;
                            inFrame->sample_rate    = cdc_ctx_in->sample_rate;
                        }
                        catch(...)
                        {
                            throw ExceptionClass("sliceprocess", "thrProcessa", "erro ao tentar atualizar informacoes o codec context do objRadio.");
                        }

/**/
                        // rodando a thread do fingerprint
                        try
                        {
                            objRawData->setBuffer(arqName + ".wav", Packets, inFrameSize, cdc_ctx_in->sample_fmt,
                                                  cdc_ctx_in->sample_rate, objRadio->getChannelLayout());
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
                            objFileData->setBuffer(arqName + ".mp3", Packets, inFrameSize, cdc_ctx_in->sample_fmt,
                                                  cdc_ctx_in->sample_rate, objRadio->getChannelLayout());
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

//objLog->mr_printf(MR_LOG_DEBUG, idRadio, "Recorte : %5d    Fifo : %d\n", idSlice, objRadio->getQueueSize());
                        idSlice++;

                        sleep(1);
                    }
                }
            }
            catch(SignalException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro de segmentacao\n");
            }
            catch(ExceptionClass& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "%s\n", err.what());
            }
            catch(FifoException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "FIFO %d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(ConvertException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro na abertura de arquivo : %d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(StreamException& err)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro na abertura de arquivo : %d\n", *boost::get_error_info<errno_code>(err));
            }
            catch(...)
            {
                objLog->mr_printf(MR_LOG_ERROR, idRadio, "General Error\n");
            }
        }
    }
    catch(SignalException& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "sliceprocess (thrProcessa) : Erro de segmentacao geral\n");
    }
    catch(BadAllocException& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro de alocacao de memoria : %d\n", *boost::get_error_info<errno_code>(err));
    }
    catch(RawDataException& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro na criacao do objeto RawData : %d\n", *boost::get_error_info<errno_code>(err));
    }
    catch(FileDataException& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Erro na criacao do objeto FileData : %d\n", *boost::get_error_info<errno_code>(err));
    }
    catch (ExceptionClass& err)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, err.what());
    }
    catch(...)
    {
        Status = enumSliceProcess::ERROR;
        objLog->mr_printf(MR_LOG_ERROR, idRadio, ">>>>>>>>>>>>>>>>>>> Erro grave, sem indice. <<<<<<<<<<<<<<<<<<<\n");
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
