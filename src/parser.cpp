#include "parser.h"

Parser::Parser()
{
    isExit = false;
    lockFifo = true;
    cntRawDayCut = 0;
    cntM4aDayCut = 0;
}

Parser::~Parser()
{
    isExit = true;

    sleep(2);
}

// somente para teste
static char *const get_error_text(const int error)
{
    static char error_buffer[255];
    av_strerror(error, error_buffer, sizeof(error_buffer));
    return error_buffer;
}

AVFormatContext* Parser::getFormatContext(bool isRAW)
{
    if (isRAW)
        return rawFormatContext;
    else
        return m4aFormatContext;
}

AVCodecContext* Parser::getCodecContext(bool isRAW)
{
    if (isRAW)
        return rawCodecContext;
    else
        return m4aCodecContext;
}

AVCodecContext* Parser::getInCodecContext()
{
    return inCodecContext;
}

SwrContext* Parser::getSwrContext(bool isRAW)
{
    if (isRAW)
        return rawSwrContext;
    else
        return m4aSwrContext;
}

unsigned char* Parser::ReadData(AVAudioFifo *fifo)
{
    bufRaw.push_back((unsigned char)'a');

    return NULL;
}

void Parser::CreateContext(string arqName, bool isRaw, AVDictionary* options)
{
    try
    {
        if (isRaw)
        {
            rawFormatContext = CreateFormatContext(arqName, isRaw);
            rawCodecContext = CreateCodecContext(rawFormatContext, 1, 44100, AVSampleFormat::AV_SAMPLE_FMT_S16, &options);
            rawCodecContext->frame_size = 1024;
            rawSwrContext = CreateSwrContext(inCodecContext, rawCodecContext);
        }
        else
        {
            m4aFormatContext = CreateFormatContext(arqName, isRaw);
            m4aCodecContext = CreateCodecContext(m4aFormatContext, 1, 44100, AVSampleFormat::AV_SAMPLE_FMT_S16, &options);
            m4aSwrContext = CreateSwrContext(inCodecContext, m4aCodecContext);
        }
    }
    catch(...)
    {
        throw;
    }
}

int Parser::EncodeFrames(bool isRAW)
{
    int haveData = 0;

    AVCodecContext* CodecContext;
    SwrContext* swrContext;
    AVFrame* inFrame;
    AVFrame* outFrame;
    AVPacket* outPacket;

    if (isRAW)
    {
        CodecContext = rawCodecContext;
        swrContext = rawSwrContext;
        inFrame = rawInFrame;
        outFrame = rawOutFrame;
        outPacket = &rawOutPacket;
    }
    else
    {
        CodecContext = m4aCodecContext;
        swrContext = m4aSwrContext;
        inFrame = m4aInFrame;
        outFrame = m4aOutFrame;
        outPacket = &m4aOutPacket;
    }

    outFrame->nb_samples     = CodecContext->frame_size;
    outFrame->channel_layout = CodecContext->channel_layout;
    outFrame->format         = CodecContext->sample_fmt;
    outFrame->sample_rate    = CodecContext->sample_rate;
    av_frame_get_buffer(outFrame, 0);

    swr_convert(swrContext, (uint8_t**)outFrame->extended_data, outFrame->nb_samples, (const uint8_t**)inFrame->data, inFrame->nb_samples);

    av_init_packet(outPacket);
    outPacket->data = NULL;
    outPacket->size = 0;

    int ret = avcodec_encode_audio2(CodecContext, outPacket, outFrame, &haveData);

    av_frame_unref(outFrame);

    return haveData;
}

void Parser::ProcessFrames()
{
    int inFrameSize = inCodecContext->frame_size;
    int szFifo;
    int ctrCnt = 0;
    vector <uint8_t> binOutput;
    arqData_ arqData;

    while ((maxFrames = objRadio->getNumFrames(5)) == 0);

    rawOutFrame = av_frame_alloc();
    rawInFrame = av_frame_alloc();

clock_t start;
start = clock();

    sprintf(arqData.arqName, "%05d-%05d-%s", this->idRadio, cntRawDayCut, getDateTime().c_str());
    initFIFO();

    while(!isExit)
    {
        int data_present = 0;

        szFifo = objRadio->getFifoSize();
        if (szFifo >= inFrameSize)
        {
// carregando dados da FIFO
            rawInFrame->nb_samples     = FFMIN(szFifo, inFrameSize);
            rawInFrame->channel_layout = inCodecContext->channel_layout;
            rawInFrame->format         = inCodecContext->sample_fmt;
            rawInFrame->sample_rate    = inCodecContext->sample_rate;
            av_frame_get_buffer(rawInFrame, 0);

            objRadio->getFifoData((void**)rawInFrame->data, rawInFrame->nb_samples);

// conversao para AAC
            addSamplesFIFO((uint8_t**)&rawInFrame->data, rawInFrame->nb_samples);

// conversao para RAW
            if (EncodeFrames(true))
            {
                for (int i = 0; i < rawOutPacket.size; i++)
                    binOutput.push_back(rawOutPacket.data[i]);
            }

            av_free_packet(&rawOutPacket);

// Controle de tempo, maxFrames tem a quantidade de frames necessarios para os recortes
            ctrCnt++;

            if (ctrCnt >= maxFrames)
            {
                cntRawDayCut++;
                ctrCnt = 0;
                while ((maxFrames = objRadio->getNumFrames(5)) == 0);    // atualizando a quantidade max de frames

                uint8_t* conv;
                int pos = 0;
                int freq = inCodecContext->sample_rate;

// gerando fingerprints
                unsigned int nbits;
                unsigned int* bits = CreateFingerPrint(binOutput, &nbits, true);
                binOutput.clear();

                string strDateTime = getDateTime();

//                Database* objDb = new Database(sqlConnString);
//                long idSlice = objDb->insertCutHistory(this->idRadio, getSaveCutDir(), strDateTime);
                long idSlice = cntRawDayCut;

                uint8_t* buff  = new uint8_t[4 * nbits + 16];

                conv = (uint8_t*)&this->idRadio;
                for (int i = 0; i < 4; buff[pos++] = conv[i++]);

                conv = (uint8_t*)&idSlice;
                for (int i = 0; i < 4; buff[pos++] = conv[i++]);

                conv = (uint8_t*)&freq;
                for (int i = 0; i < 4; buff[pos++] = conv[i++]);

                conv = (uint8_t*)&nbits;
                for (int i = 0; i < 4; buff[pos++] = conv[i++]);

                for (int j = 0; j < nbits; j++)
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

                    TCPClient* objClient = new TCPClient(IO_Service, EndPointIterator);

                    boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &IO_Service));
                    ClientThread.join();

                    objClient->Send(buff, pos);
                }
                catch(ConvertException& e)
                {
                    cerr << e.what() << endl;
                }
                catch (exception& e)
                {
                    cerr << e.what() << endl;
                }

                delete[] buff;

cout << "Radio : " << this->idRadio << "    Recorte : " << cntRawDayCut << "    Size : " << szFifo - inFrameSize << "    Tempo de processamento : " << (float)(clock() - start)/CLOCKS_PER_SEC << endl;
start = clock();
            }

            av_frame_unref(rawInFrame);
        }
    }

    av_frame_free(&rawInFrame);
    av_frame_free(&rawOutFrame);
}

void Parser::SetStreamRadio(unsigned int idxRadio, StreamRadio* oRadio)
{
    idRadio = idxRadio;
    objRadio = oRadio;

    inCodecContext = objRadio->getCodecContext();
}

AVFormatContext* Parser::CreateFormatContext(string arqName, bool isRaw)
{
    AVIOContext *ioContext = NULL;
    AVFormatContext *outFormatContext = NULL;

    if (!(outFormatContext = avformat_alloc_context()))
        throw ConvertException() << errno_code(MIR_ERR_OPEN_FORMAT_CONTEXT);

    if (!(outFormatContext->oformat = av_guess_format(NULL, arqName.c_str(), NULL)))
        throw ConvertException() << errno_code(MIR_ERR_OPEN_OUTPUT_FORMAT);

    if (isRaw)
    {
        outFormatContext->oformat->flags |= AVFMT_NOFILE;
    }
    else
    {
        outFormatContext->oformat->flags |= AVFMT_ALLOW_FLUSH;

        if (avio_open(&ioContext, arqName.c_str(), AVIO_FLAG_WRITE) < 0)
            throw ConvertException() << errno_code(MIR_ERR_OPEN_OUTPUT_FILE);
        outFormatContext->pb = ioContext;


        int len;
        for (len = 0; (arqName.c_str())[len] != 0; outFormatContext->filename[len] = (arqName.c_str())[len++]);
        outFormatContext->filename[len] = 0;
    }

    return outFormatContext;
}

AVCodecContext* Parser::CreateCodecContext(AVFormatContext* frmContext, int channel, int SampleRate, AVSampleFormat SampleFormat, AVDictionary** outOptions)
{
    AVCodec *outCodec = NULL;
    AVStream *outStream = NULL;
    AVCodecContext* outCodecContext = NULL;

    if (!(outCodec = avcodec_find_encoder(frmContext->oformat->audio_codec)))
        throw ConvertException() << errno_code(MIR_ERR_OPEN_CODEC);
    if (!(outStream = avformat_new_stream(frmContext, outCodec)))
        throw ConvertException() << errno_code(MIR_ERR_OPEN_STREAM);
    outCodecContext = outStream->codec;

    outCodecContext->channels       = channel;
    outCodecContext->channel_layout = av_get_default_channel_layout(channel);
    outCodecContext->sample_rate    = SampleRate;
    outCodecContext->sample_fmt     = SampleFormat;

    if (frmContext->oformat->flags & AVFMT_GLOBALHEADER)
        outCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;

    if ((avcodec_open2(outCodecContext, outCodec, outOptions)) < 0)
        throw ConvertException() << errno_code(MIR_ERR_OPEN_CODEC_CONTEXT);

    return outCodecContext;
}

SwrContext* Parser::CreateSwrContext(AVCodecContext *inCodecContext, AVCodecContext *outCodecContext)
{
    SwrContext* swrContext = swr_alloc_set_opts(NULL,
       av_get_default_channel_layout(outCodecContext->channels),
       outCodecContext->sample_fmt,
       outCodecContext->sample_rate,
       av_get_default_channel_layout(inCodecContext->channels),
       inCodecContext->sample_fmt,
       inCodecContext->sample_rate,
       0, NULL);

    if (!swrContext)
        throw ContextCreatorException() << errno_code(MIR_ERR_ALLOC_SWR_CONTEXT);

    /** Open the resampler with the specified parameters. */
    if (swr_init(swrContext) < 0)
        throw ContextCreatorException() << errno_code(MIR_ERR_ALLOC_SWR_CONTEXT);

    return swrContext;
}

string Parser::getDateTime()
{
    struct tm *DtHr;
    time_t t;
    char strAux[20];

    t = time(NULL);
    DtHr = localtime(&t);

    sprintf(strAux, "%02d%02d%02d_%02d%02d%02d", DtHr->tm_year - 100, DtHr->tm_mon + 1, DtHr->tm_mday, DtHr->tm_hour, DtHr->tm_min, DtHr->tm_sec);

    return strAux;
}

string Parser::getDate()
{
    struct tm *DtHr;
    time_t t;
    char strAux[20];

    t = time(NULL);
    DtHr = localtime(&t);

    sprintf(strAux, "%02d%02d%02d", DtHr->tm_year - 100, DtHr->tm_mon + 1, DtHr->tm_mday);

    return strAux;
}

string Parser::getTime()
{
    struct tm *DtHr;
    time_t t;
    char strAux[20];

    t = time(NULL);
    DtHr = localtime(&t);

    sprintf(strAux, "%02d", DtHr->tm_hour);

    return strAux;
}

string Parser::getSaveCutDir()
{
    DIR *dir;
    char strAux[10];

    string ret = cutFolder + "/" + getDate();
    if ((dir = opendir(ret.c_str())) == NULL)
    {
        cntRawDayCut = 0;
        cntM4aDayCut = 0;
        mkdir(ret.c_str(), 0777);
    }
    else
        closedir(dir);

    ret += "/" + getTime();
    if ((dir = opendir(ret.c_str())) == NULL)
        mkdir(ret.c_str(), 0777);
    else
        closedir(dir);

    sprintf(strAux, "%05d", this->idRadio);
    ret += "/" + string(strAux);

    if ((dir = opendir(ret.c_str())) == NULL)
        mkdir(ret.c_str(), 0777);
    else
        closedir(dir);

    return ret;
}

unsigned int* Parser::CreateFingerPrint(vector <uint8_t> Data, unsigned int* FingerPrintSize, bool mltFFT)
{
    FingerPrint* fingerPrint = new FingerPrint(Filters);
    unsigned int len = Data.size();

    uint8_t* convArray = (uint8_t*)calloc(1, len);
    for (unsigned int i = 0; i < len; i += 2)
    {
        convArray[i] = Data[i + 1] & 0xff;
        convArray[i + 1] = Data[i] & 0xff;
    }

    unsigned int *bits = fingerPrint->Wav2Bits((short int*)convArray, len / 2, 44100, FingerPrintSize, mltFFT);
    free(convArray);

    return bits;
}

void Parser::initFIFO()
{
    /** Create the FIFO buffer based on the specified output sample format.
       nb_samples  initial allocation size, in samples */


    if (!(arqFifo = av_audio_fifo_alloc(inCodecContext->sample_fmt, inCodecContext->channels, 1)))
    {
        printf("FIFO não pode ser alocado.\n");
        throw BadAllocException() << errno_code(AVERROR(ENOMEM));
    }

    lockFifo = false;
}

void Parser::addSamplesFIFO(uint8_t **inputSamples, const int frameSize)
{
    int error;

    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
    while (lockFifo);
    lockFifo = true;

    if ((error = av_audio_fifo_realloc(arqFifo, av_audio_fifo_size(arqFifo) + frameSize)) < 0)
    {
        printf("Could not reallocate FIFO\n");
    }

    /** Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(arqFifo, (void **)inputSamples,
                            frameSize) < frameSize)
    {
        printf("Could not write data to FIFO\n");
    }

    lockFifo = false;
}

int Parser::getFifoData(void **data, int nb_samples)
{
    while (lockFifo);
    lockFifo = true;

    int ret = av_audio_fifo_read(arqFifo, data, nb_samples);

    lockFifo = false;

    return ret;
}

int Parser::getFifoSize()
{
    while (lockFifo);
    lockFifo = true;
    int ret = av_audio_fifo_size(arqFifo);
    lockFifo = false;
    return ret;
}

void Parser::ProcessOutput()
{
    arqData_ arqData;
    string arqAtual = "";
    string arqName;
    char chrArqName[28];

    int ctrCnt = 1000000;

    m4aOutFrame = av_frame_alloc();
    m4aInFrame = av_frame_alloc();

    while (true)
    {
        try
        {
            if (getFifoSize() > 0)
            {
                m4aInFrame->nb_samples     = inCodecContext->frame_size;
                m4aInFrame->channel_layout = inCodecContext->channel_layout;
                m4aInFrame->format         = inCodecContext->sample_fmt;
                m4aInFrame->sample_rate    = inCodecContext->sample_rate;
                av_frame_get_buffer(m4aInFrame, 0);

                getFifoData((void**)m4aInFrame->data, inCodecContext->frame_size);
    //            getFifoData((void**)&arqData, inCodecContext->frame_size + 26);
    //            arqName = arqData.arqName;

    //            if (arqAtual != arqFifo)
                if (ctrCnt >= maxFrames)
                {
                    ctrCnt = 0;
                    // novo recorte, fecha o atual e abre o novo
                    if (arqAtual != "")
                    {
                        av_write_trailer(m4aFormatContext);

                        av_free(m4aFormatContext);
                        av_free(m4aCodecContext);
                        av_free(m4aSwrContext);
                    }

                    cntM4aDayCut++;
                    sprintf(chrArqName, "%05d-%05d-%s", this->idRadio, cntM4aDayCut, getDateTime().c_str());
                    arqName = chrArqName;

                    CreateContext(getSaveCutDir() + "/" + arqName + ".mp3", false, NULL);
                    avformat_write_header(m4aFormatContext, NULL);

                    arqAtual = arqName;
                }

                if (EncodeFrames(false))
                {
                    av_write_frame(m4aFormatContext, &m4aOutPacket);
                }
                ctrCnt++;

                av_free_packet(&m4aOutPacket);
            }
        }
        catch(ConvertException& ex)
        {
            cerr << ex.what() << endl;
        }
        catch(...)
        {
            cout << "ProcessOutput" << endl;
        }
    }
}
