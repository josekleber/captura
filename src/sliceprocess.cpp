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
}

SliceProcess::~SliceProcess()
{
    stopThread = true;
}

void SliceProcess::thrProcessa()
{
    int szFifo;
    int inFrameSize = objRadio->getCodecContext()->frame_size;

    RAWData* objRawData;
    FileData* objFileData;

    AVFrame* inFrame = av_frame_alloc();

    AVCodecContext* cdc_ctx_in = objRadio->getCodecContext();

    idSlice = 0;

    try
    {
        inFrame->channel_layout = cdc_ctx_in->channel_layout;
        inFrame->format         = cdc_ctx_in->sample_fmt;
        inFrame->sample_rate    = cdc_ctx_in->sample_rate;
    }
    catch(...)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "catch open connection." ANSI_COLOR_RESET;
        status = -1;
        return;
        //throw;
    }
    //boost::this_thread::sleep(boost::posix_time::microseconds(5));

    try
    {
        int qtdPack = 0;

        vector <vector<uint8_t>> Dados;

        while (!stopThread)
        {
            szFifo = objRadio->getFifoSize();

            if (szFifo >= inFrameSize)
            {
                int szFrame = FFMIN(szFifo, inFrameSize);

                inFrame->nb_samples     = szFrame;
                av_frame_get_buffer(inFrame, 0);

                // carregando dados da FIFO
                objRadio->getFifoData((void**)inFrame->data, szFrame);

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
                    arqName = getSaveCutDir() + "/" + arqName;
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
                        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "catch do objParser" ANSI_COLOR_RESET;
                        status = -1;
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
                        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "catch do objParser" ANSI_COLOR_RESET;
                        status = -1;
                        throw;
                    }

                    // reseta dados
                    for (int i = 0; i < Dados.size(); i++)
                        (Dados[i]).clear();
                    Dados.clear();
                }
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

    return path;
}
