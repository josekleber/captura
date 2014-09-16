#include "streamradio.h"


StreamRadio::StreamRadio()
{
    // zera o timer
    timer = 0;
    formatContext = NULL;
    codecContext = NULL;
    stream = NULL;
    dictionary = NULL;
    fifo = NULL;
    duration = 0;
    statusConnection = MIR_CONNETION_CLOSE;
    isExit = false;
    bitRate = 0;
    isVBR = false;
    mtx_.unlock();

    // define o nível de log do FFMPEG
    av_log_set_level(AV_LOG_ERROR);
}

StreamRadio::~StreamRadio()
{
    //dtor
    if ((formatContext))
        avformat_free_context(formatContext);

    isExit = true;

    boost::this_thread::sleep(boost::posix_time::seconds(10));
}

double StreamRadio::getConnectionTime()
{
    if ((statusConnection != MIR_CONNECTION_OPEN))
        return 0;

    clock_t now = clock();

    double elapsed_secs = double(now - timer) / CLOCKS_PER_SEC;

    return elapsed_secs;
}

EnumStatusConnect StreamRadio::getStatus()
{
    return statusConnection;
}

void StreamRadio::close()
{
    if ((formatContext))
        avformat_close_input(&formatContext);

    timer = 0;
    statusConnection = MIR_CONNETION_CLOSE;
}

AVFormatContext* StreamRadio::open(string uri)
{
    formatContext = avformat_alloc_context();
    int ret = 0; // retorno das funções FFMPEG

    // verifica se conseguiu alocar o contexto
    if (!(formatContext))
        throw BadAllocException() << errno_code(MIR_ERR_BADALLOC_CONTEXT);

    // usa todos os cores do processador.
    addOptions("threads","0");

    // força RTSP usar TCP
    rtspDetect(uri);

    // abre a conexão
    if ((ret=avformat_open_input(&formatContext,uri.c_str(),NULL,&dictionary))< 0)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro de conexão com o stream " << uri.c_str();
        statusConnection = MIR_CONNECTION_ERROR;
        throw OpenConnectionException() <<errno_code(MIR_ERR_STREAM_CONNECTION);
    }

    // pega informações do stream
    if ((ret=avformat_find_stream_info(formatContext,NULL)) < 0)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro ao tentar pegar informações dos streams de entrada: " << uri.c_str();
        statusConnection = MIR_CONNECTION_ERROR;
        throw OpenConnectionException() <<errno_code(MIR_ERR_STREAM_CONNECTION);
    }

    BOOST_LOG_TRIVIAL(info) << ANSI_COLOR_GREEN "Conectado ao stream "  << uri.c_str() << ANSI_COLOR_RESET;

    statusConnection = MIR_CONNECTION_OPEN;

    setStreamType(); // pega os streams que a conexão contém

    timer = clock(); // inicia a contagem do tempo de conexão

    return formatContext;
}

void StreamRadio::read()
{
    if (statusConnection != MIR_CONNECTION_OPEN)
        throw ConnectionClosedException() <<errno_code(MIR_ERR_CONNECTION_CLOSED);

    readFrame();
}

void StreamRadio::setStreamType()
{
    if ((statusConnection != MIR_CONNECTION_OPEN))
        throw ConnectionClosedException() <<errno_code(MIR_ERR_CONNECTION_CLOSED);

    // vetor com a quantidade de streams da conexão
    streamType = new StreamType[formatContext->nb_streams];

    for (unsigned int index = 0; index < formatContext->nb_streams; index++)
    {
        streamType[index].id = index;
        streamType[index].streamType = formatContext->streams[index]->codec->codec_type;

        // mesmo que haja mais de um stream de audio, neste momento, considero apenas o primeiro.
        if (!(stream))
        {
            if (streamType[index].streamType == AVMEDIA_TYPE_AUDIO)
            {
                stream = formatContext->streams[index];

                if (stream->codec->codec_id == AV_CODEC_ID_WMAPRO)
                    codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
                else
                    codec = avcodec_find_decoder(stream->codec->codec_id);

                BOOST_LOG_TRIVIAL(info) << "Codec de entrada " << codec->name;

                codecContext = avcodec_alloc_context3(codec);
                codecContext = stream->codec;

                if (avcodec_open2(codecContext,codec,NULL) < 0)
                {
                    BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro de alocação de contexto " << codec->name;
                    throw BadAllocException() << errno_code(MIR_ERR_BADALLOC_CONTEXT);
                }

                if (codecContext->bit_rate == 0)
                {
                    BOOST_LOG_TRIVIAL(info) << "VBR detectado.";
                    isVBR = true;
                }
                else
                    bitRate = codecContext->bit_rate;

                if ((codecContext->codec == NULL))
                {
                    BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Codec não suportado.";
                    throw CodecNotSupportedException() <<errno_code(MIR_ERR_CODEC_NOT_SUPPORTED);
                }
            }
            else if (formatContext->streams[index]->codec->coder_type == AVMEDIA_TYPE_ATTACHMENT)
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_CYAN "Media type : AVMEDIA_TYPE_ATTACHMENT";
            else if (formatContext->streams[index]->codec->coder_type == AVMEDIA_TYPE_DATA)
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_CYAN "Media type : AVMEDIA_TYPE_DATA";
            else if (formatContext->streams[index]->codec->coder_type == AVMEDIA_TYPE_NB)
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_CYAN "Media type : AVMEDIA_TYPE_NB";
            else if (formatContext->streams[index]->codec->coder_type == AVMEDIA_TYPE_SUBTITLE)
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_CYAN "Media type : AVMEDIA_TYPE_SUBTITLE";
            else if (formatContext->streams[index]->codec->coder_type == AVMEDIA_TYPE_VIDEO)
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_CYAN "Media type : AVMEDIA_TYPE_VIDEO";
            else
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_CYAN "Media type : AVMEDIA_TYPE_UNKNOW";
        }
    }

    // se chegar aqui e não tiver o stream é sinal de que não existe na conexão o AVMEDIA_TYPE_AUDIO
    // neste caso lanço a exceção
    if (!(stream))
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Não há stream de áudio na conexão. Stream usado" ANSI_COLOR_RESET << formatContext->filename;
        throw MediaTypeNoAudioException() << errno_code(MIR_ERR_MEDIA_TYPE_NO_AUDIO);
    }


}

void StreamRadio::addOptions(string key, string value)
{
    ///TODO: adicionar o controle de FLAGS. falta estudo de sua aplicação
    av_dict_set(&dictionary,key.c_str(),value.c_str(),0);
}

AVDictionary * StreamRadio::getListOptions()
{
    return dictionary;
}

StreamType * StreamRadio::getStreamType()
{
    if ((statusConnection != MIR_CONNECTION_OPEN))
        throw ConnectionClosedException() <<errno_code(MIR_ERR_CONNECTION_CLOSED);

    return streamType;
}

AVCodecContext * StreamRadio::getCodecContext()
{
    return codecContext;
}

AVStream * StreamRadio::getStream()
{
    return stream;
}

void StreamRadio::rtspDetect(string uri)
{
    int pos = uri.find("rtsp://",0);

    if ((pos >= 0))
    {
        BOOST_LOG_TRIVIAL(info) << ANSI_COLOR_YELLOW "Usando TCP com protocolo RTSP." << ANSI_COLOR_RESET;
        av_dict_set(&dictionary,"rtsp_transport","tcp",0);
    }
}

void StreamRadio::initFIFO()
{
    /** Create the FIFO buffer based on the specified output sample format.
       nb_samples  initial allocation size, in samples */


    if (!(this->fifo = av_audio_fifo_alloc(codecContext->sample_fmt, codecContext->channels,1)))
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "FIFO não pode ser alocado." ANSI_COLOR_RESET;
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Radio-> " << this->formatContext->filename << ANSI_COLOR_RESET;
        throw BadAllocException() <<errno_code(AVERROR(ENOMEM));
    }
}

void StreamRadio::readFrame()
{
    bool isTrue = true;
    int finished = 0;
    int error;
    int data;

    // Pacote para dados temporários.
    AVPacket inputPacket;

    // inicia as FIFOS
    initFIFO();

    while (isTrue && (finished == 0) && !isExit)
    {
        frame = NULL;
        av_init_packet(&inputPacket);
        // ainda não entendi o porquê setar os dados abaixo. Porém, em todos os
        // exemplos da ferramenta foi inicializado então...
        inputPacket.data = NULL;
        inputPacket.size = 0;

        try
        {
            if (!(frame = av_frame_alloc()))
            {
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro de alocação do frame." ANSI_COLOR_RESET;
                //throw BadAllocException() <<errno_code(MIR_ERR_BADALLOC_CONTEXT);
            }

            // Lê um frame do áudio e coloca no pacote temporário.
            if ((error = av_read_frame(formatContext, &inputPacket)) < 0)
            {
                // Se for final de arquivo ( em caso de arquivo ).
                if (error == AVERROR_EOF)
//                break;
                    finished = 1;
                else
                {
                    BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro de leitura do frame." ANSI_COLOR_RESET;
                    //throw FrameReadException() <<errno_code(MIR_ERR_FRAME_READ);
                }
            }

            if (!finished)
            {
                try
                {
                    BOOST_LOG_TRIVIAL(debug) <<  "Decode do frame.";

                    // decodifica o frame
                    decodeAudioFrame(&data,&finished,&inputPacket);

                    /**
                    * Se o decodificador não terminar de processar os dados, esta função será chamada novamente.
                    */
                    if (finished && data)
                        finished = 0;

                    // libera memória do pacote temporário
                    av_free_packet(&inputPacket);

                    // Adiciona ao FIFO
                    addSamplesFIFO(frame->extended_data, frame->nb_samples);
                }
                catch(DecoderException ex)
                {
                    BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro de decoding." ANSI_COLOR_RESET;
                    av_free_packet(&inputPacket);
                }

                //boost::this_thread::sleep(boost::posix_time::milliseconds(1));
            }
        }
        catch (...)
        {
            BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Ocorreu um erro no stream de entrada." ANSI_COLOR_RESET;
        }
    }
}

void StreamRadio::decodeAudioFrame(int *data, int *finished, AVPacket *inputPacket)
{
    int error=0;

    /**
     * Decodificar o frame de áudio armazenados no pacote temporário.
     * Se estamos no final do arquivo, passar um pacote vazio para o descodificador
     * para nivelá-lo.
     */
    if ((error = avcodec_decode_audio4(codecContext, frame,
                                       data, inputPacket)) < 0)
    {
        printf("Could not decode frame (error '%d')\n", error);
        throw DecoderException() << errno_code(MIR_ERR_DECODE);
    }

    if (duration == 0)
    {
        double time_base = av_q2d(formatContext->streams[0]->time_base);
        duration = time_base * av_frame_get_pkt_duration(frame);
    }
}

void StreamRadio::addSamplesFIFO(uint8_t **inputSamples, const int frameSize)
{
    int error;

    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
    mtx_.lock();

    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frameSize)) < 0)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Não foi possível alocar o FIFO." ANSI_COLOR_RESET;
    }

    /** Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)inputSamples,
                            frameSize) < frameSize)
    {
        printf("Could not write data to FIFO\n");
    }
    mtx_.unlock();
}

AVAudioFifo** StreamRadio::getFIFO()
{
    return &fifo;
}

int StreamRadio::getFifoSize()
{
    mtx_.lock();
    int ret = av_audio_fifo_size(fifo);
    mtx_.unlock();
    return ret;
}
int StreamRadio::getFifoData(void **data, int nb_samples)
{
    mtx_.lock();
    int ret = av_audio_fifo_read(fifo, data, nb_samples);
    mtx_.unlock();
    return ret;
}

int StreamRadio::getNumFrames(double sec)
{
    if (duration != 0)
        return (int)round(sec / duration + 0.5);
    else
        return 0;
}
