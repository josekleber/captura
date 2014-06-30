#include "parser.h"

Parser::Parser()
{
    isExit = false;
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

int Parser::EncodeFrames(bool isRAW)
{
    int haveData = 0;

    AVCodecContext* CodecContext;
    SwrContext* swrContext;

    if (isRAW)
    {
        CodecContext = rawCodecContext;
        swrContext = rawSwrContext;
    }
    else
    {
        CodecContext = m4aCodecContext;
        swrContext = m4aSwrContext;
    }

    outFrame->nb_samples     = CodecContext->frame_size;
    outFrame->channel_layout = CodecContext->channel_layout;
    outFrame->format         = CodecContext->sample_fmt;
    outFrame->sample_rate    = CodecContext->sample_rate;
    av_frame_get_buffer(outFrame, 0);

    swr_convert(swrContext, (uint8_t**)outFrame->extended_data, outFrame->nb_samples, (const uint8_t**)inFrame->data, inFrame->nb_samples);

    av_init_packet(&outPacket);
    outPacket.data = NULL;
    outPacket.size = 0;

    int ret = avcodec_encode_audio2(CodecContext, &outPacket, outFrame, &haveData);

    av_frame_unref(outFrame);

    return haveData;
}

void Parser::ProcessFrames()
{
    int inFrameSize = inCodecContext->frame_size;
    int szFifo;
    int ctrCnt = 0;
    vector <uint8_t> binOutput;
    int maxFrames = objRadio->getNumFrames(6);

    outFrame = av_frame_alloc();
    inFrame = av_frame_alloc();

string PastaRaiz;
string arqDateTime;
PastaRaiz = "/home/nelson/Projetos/Musicas/Recortes/";
arqDateTime = getDateTime();
int cnt = 0;
int qntRecortes = 0;
clock_t start;
start = clock();
int finished = 0;

//do
//{
//    szFifo = objRadio->getFifoSize();
//    cout << szFifo << endl;
//    sleep(2);
//} while(szFifo != objRadio->getFifoSize());

    while(!isExit)
    {
        int data_present = 0;

        szFifo = objRadio->getFifoSize();
        if (szFifo >= inFrameSize)
        {
// carregando dados da FIFO
            inFrame->nb_samples     = FFMIN(szFifo, inFrameSize);
            inFrame->channel_layout = inCodecContext->channel_layout;
            inFrame->format         = inCodecContext->sample_fmt;
            inFrame->sample_rate    = inCodecContext->sample_rate;
            av_frame_get_buffer(inFrame, 0);

            objRadio->getFifoData((void**)inFrame->data, inFrame->nb_samples);
cnt++;
//cout << "Pacote : " << cnt << "    Size : " << szFifo - inFrameSize << endl;

// conversao para RAW
            EncodeFrames(true);

            if (EncodeFrames(true))
            {
                for (int i = 0; i < outPacket.size; i++)
                    binOutput.push_back(outPacket.data[i]);
            }

            av_free_packet(&outPacket);

//// conversao para AAC
//            if (EncodeFrames(false))
//            {
//                av_write_frame(m4aFormatContext, &outPacket);
//            }
//
//            av_free_packet(&outPacket);


// Controle de tempo, maxFrames tem a quantidade de frames necessarios para os recortes
            ctrCnt++;
            if (ctrCnt >= maxFrames)
            {
qntRecortes++;
                ctrCnt = 0;

// gerando fingerprints
                unsigned int nbits;
                unsigned int pp = binOutput.size();
                unsigned int* bits = CreateFingerPrint(Filters, binOutput, &nbits, mutex_acesso, true);
                binOutput.clear();

std::ofstream outfile (PastaRaiz + "Fingerprints/" + to_string(qntRecortes) + "-66_out_fp_" + arqDateTime + ".bin",std::ofstream::binary);
outfile.write ((char*)bits, nbits * sizeof(unsigned int));
outfile.close();

arqDateTime = getDateTime();

//// finalizando arquivo AAC
//                av_write_trailer(m4aFormatContext);
//
//                av_free(m4aFormatContext);
//                av_free(m4aCodecContext);
//
//
//                m4aFormatContext = CreateFormatContext(PastaRaiz + "Musicas/" + to_string(qntRecortes + 1) + "-66_out_" + arqDateTime + ".aac", false);
//                m4aCodecContext = CreateCodecContext(m4aFormatContext, 1, 44100, AVSampleFormat::AV_SAMPLE_FMT_S16, NULL);
//                avformat_write_header(m4aFormatContext, NULL);

cout << "Recorte : " << qntRecortes << "    Pacote : " << cnt << "    Size : " << szFifo - inFrameSize << "    Tempo de processamento : " << (float)(clock() - start)/CLOCKS_PER_SEC << endl;
start = clock();
            }

            av_frame_unref(inFrame);
        }
/**
        else
        {
            finished = 1;
qntRecortes++;

// finalizando o ultimo fingerprint
            unsigned int nbits;
            unsigned int *bits = CreateFingerPrint(Filters, binOutput, &nbits, mutex_acesso, true);

            std::ofstream outfile (PastaRaiz + "Fingerprints/" + to_string(qntRecortes) + "-66_out_fp_" + arqDateTime + ".bin",std::ofstream::binary);
            outfile.write ((char*)bits, nbits * sizeof(unsigned int));
            outfile.close();

//// limpando fila do AAC
//do
//{
//    av_init_packet(&outPacket);
//    outPacket.data = NULL;
//    outPacket.size = 0;
//
//    avcodec_encode_audio2(m4aCodecContext, &outPacket, NULL, &data_present);
//
//    if (data_present)
//    {
//        av_write_frame(m4aFormatContext, &outPacket);
//    }
//
//    av_free_packet(&outPacket);
//} while (data_present);
//
//            av_write_trailer(m4aFormatContext);

cout << "Recorte : " << qntRecortes << "    Pacote : " << cnt << "    Size : " << szFifo << "    Tempo de processamento : " << (float)(clock() - start)/CLOCKS_PER_SEC << endl;
        }
/**/
    }

    av_frame_free(&inFrame);
    av_frame_free(&outFrame);

cout << "Fim" << endl;
}

void Parser::SetStreamRadio(StreamRadio* oRadio)
{
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

    sprintf(strAux, "%d%02d%02d_%02d%02d%02d", DtHr->tm_year + 1900, DtHr->tm_mon + 1, DtHr->tm_mday, DtHr->tm_hour, DtHr->tm_min, DtHr->tm_sec);

    return strAux;
}

unsigned int* Parser::CreateFingerPrint(vector<Filter> Filters, vector <uint8_t> Data, unsigned int* FingerPrintSize, pthread_mutex_t* MutexAccess, bool mltFFT)
{
    FingerPrint* fingerPrint = new FingerPrint(Filters);
    unsigned int len = Data.size();

    uint8_t* convArray = (uint8_t*)calloc(1, len);
    for (unsigned int i = 0; i < len; i += 2)
    {
        convArray[i] = Data[i + 1] & 0xff;
        convArray[i + 1] = Data[i] & 0xff;
    }
    unsigned int *bits = fingerPrint->Wav2Bits((short int*)convArray, len / 2, 44100, FingerPrintSize, MutexAccess, mltFFT);
    free(convArray);

    return bits;
}
