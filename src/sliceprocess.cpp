#include "sliceprocess.h"

SliceProcess::SliceProcess(string ipRecognition, string portRecognition, string sqlConnString,
                           string cutFolder, int idRadio, vector<Filter> *Filters, StreamRadio* objRadio)
{
    this->ipRecognition = ipRecognition;
    this->portRecognition = portRecognition;
    this->sqlConnString = sqlConnString;
    this->cutFolder = cutFolder;
    this->idRadio = idRadio;
    this->Filters = Filters;

    this->objRadio = objRadio;

    stopThread = false;
    Status = 0;
}

SliceProcess::~SliceProcess()
{
    stopThread = true;
}

void SliceProcess::thrProcessa()
{
    AVCodecContext* cdc_ctx_in = objRadio->getCodecContext();

    int szFifo;
    int inFrameSize;

    RAWData* objRawData;
    FileData* objFileData;

    idSlice = 0;

    AVFrame* inFrame = av_frame_alloc();
    if (inFrame == NULL)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "catch open connection." ANSI_COLOR_RESET;
        Status = enumSliceProcess::ERROR;
        throw BadAllocException() << errno_code(MIR_ERR_FRAME_ALLOC);
    }

    inFrameSize             = cdc_ctx_in->frame_size;
    inFrame->channel_layout = cdc_ctx_in->channel_layout;
    inFrame->format         = cdc_ctx_in->sample_fmt;
    inFrame->sample_rate    = cdc_ctx_in->sample_rate;

    int qtdPack = 0;

    vector <vector<uint8_t>> Dados;

    while (!stopThread)
    {
        try
        {
            szFifo = objRadio->getFifoSize();

            if (szFifo >= inFrameSize)
            {
                int szFrame = FFMIN(szFifo, inFrameSize);

                inFrame->nb_samples     = szFrame;
                av_frame_get_buffer(inFrame, 0);

                // carregando dados da FIFO
                if (objRadio->getFifoData((void**)inFrame->data, szFrame) > 0)
                {
                    vector <uint8_t> aux;
                    for (int i = 0; i < inFrame->nb_samples; i++)
                        aux.push_back((uint8_t)((*inFrame->data)[i]));
                    Dados.push_back(aux);

                    qtdPack += szFrame;

                    if (qtdPack >= (inFrame->sample_rate  * 5))
                    {
                        qtdPack = 0;

                        char chrArqName[28];
                        sprintf(chrArqName, "%05d-%05d-%s", idRadio, idSlice, getDateTime().c_str());
                        string arqName = chrArqName;
                        try
                        {
                            arqName = getSaveCutDir() + "/" + arqName;
                        }
                        catch(exception& err)
                        {
                            throw;
                        }
                        // processamento para fingerprint
                        try
                        {
                            objRawData = new RAWData(arqName + ".wav", cdc_ctx_in->channel_layout,
                                                     cdc_ctx_in->sample_rate, cdc_ctx_in->bit_rate,
                                                     cdc_ctx_in->sample_fmt, szFrame, cdc_ctx_in->channels,
                                                     Filters, ipRecognition, portRecognition, idRadio, idSlice);
                            objRawData->setBuffer(Dados);
                            objThreadRawParser = new boost::thread(boost::bind(&RAWData::Execute, objRawData));
                        }
                        catch(...)
                        {
                            BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "catch do rawdata" ANSI_COLOR_RESET;
                            throw;
                        }

                        // processamento para arquivo
                        try
                        {
                            objFileData = new FileData(arqName + ".mp3", cdc_ctx_in->channel_layout,
                                                     cdc_ctx_in->sample_rate, cdc_ctx_in->bit_rate,
                                                     cdc_ctx_in->sample_fmt, szFrame, cdc_ctx_in->channels);
                            objFileData->setBuffer(Dados);
                            objThreadArqParser = new boost::thread(boost::bind(&FileData::Execute, objFileData));
                        }
                        catch(...)
                        {
                            BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "catch do filedata" ANSI_COLOR_RESET;
                            throw;
                        }

                        // reseta dados
                        for (int i = 0; i < Dados.size(); i++)
                            (Dados[i]).clear();
                        Dados.clear();

                        idSlice++;
                    }
                }
            }

            boost::this_thread::sleep(boost::posix_time::microseconds(1));
        }
        catch(exception& err)
        {
            BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "Error: " << *boost::get_error_info<errno_code>(err) << ANSI_COLOR_RESET;
        }
        catch(...)
        {
            BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED << "Error" << ANSI_COLOR_RESET;
        }
    }
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
