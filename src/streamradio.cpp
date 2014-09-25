#include "streamradio.h"

StreamRadio::StreamRadio()
{
    // zera o timer
    timer = 0;
    formatContext = NULL;
    codecContext = NULL;
    stream = NULL;
    dictionary = NULL;
    statusConnection = MIR_CONNETION_CLOSE;
    isExit = false;
    bitRate = 0;
    isVBR = false;

    // define o nível de log do FFMPEG
    av_log_set_level(AV_LOG_ERROR);
}

StreamRadio::~StreamRadio()
{
    isExit = true;

    boost::this_thread::sleep(boost::posix_time::seconds(10));

    while (statusConnection == EnumStatusConnect::MIR_CONNECTION_OPEN)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));

    if (formatContext)
        avformat_free_context(formatContext);
    if (dictionary)
        av_free(dictionary);
    if (codecContext)
        avcodec_close(codecContext);
    if (codec)
        av_free(codec);
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
    delete this;
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
    rtspDetect();

    // abre a conexão
    if ((ret=avformat_open_input(&formatContext,uri.c_str(),NULL,&dictionary))< 0)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro de conexão com o stream " << uri.c_str();
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

    // pega informações do stream
    if (avformat_find_stream_info(formatContext,NULL) < 0)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro ao tentar pegar informações dos streams de entrada: " << uri.c_str();
        statusConnection = MIR_CONNECTION_ERROR;
        throw OpenConnectionException() <<errno_code(MIR_ERR_STREAM_CONNECTION);
    }

    av_dump_format(formatContext,0,this->uri.c_str(),0);

    // recupera o melhor stream para os valores passados
    streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);

    if (streamIndex == AVERROR_STREAM_NOT_FOUND)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Não há stream de áudio na conexão. Stream usado" ANSI_COLOR_RESET << this->uri.c_str();
        throw MediaTypeNoAudioException() << errno_code(MIR_ERR_MEDIA_TYPE_NO_AUDIO);
    }
    else if (streamIndex == AVERROR_DECODER_NOT_FOUND)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Codec não suportado.";
        throw CodecNotSupportedException() <<errno_code(MIR_ERR_CODEC_NOT_SUPPORTED);
    }

    stream = formatContext->streams[streamIndex];
    codecContext = avcodec_alloc_context3(codec);
    codecContext = stream->codec;

    if (avcodec_open2(codecContext,codec,NULL) < 0)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro de alocação de contexto " << codec->name;
        throw BadAllocException() << errno_code(MIR_ERR_BADALLOC_CONTEXT);
    }

    this->channelLayout = codecContext->channel_layout;

    // se chegar aqui e não tiver o stream é sinal de que não existe na conexão o AVMEDIA_TYPE_AUDIO
    // neste caso lanço a exceção
    if (!(stream))
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Não há stream de áudio na conexão. Stream usado" ANSI_COLOR_RESET << formatContext->filename;
        throw MediaTypeNoAudioException() << errno_code(MIR_ERR_MEDIA_TYPE_NO_AUDIO);
    }


}

int StreamRadio::getChannelLayout()
{
    return channelLayout;
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

void StreamRadio::rtspDetect()
{
    int pos = this->uri.find("rtsp://",0);

    if ((pos >= 0))
    {
        BOOST_LOG_TRIVIAL(info) << ANSI_COLOR_YELLOW "Usando TCP com protocolo RTSP." << ANSI_COLOR_RESET;
        av_dict_set(&dictionary,"rtsp_transport","tcp",0);
    }
}

void StreamRadio::readFrame()
{
    bool isTrue = true;
    int finished = 0;
    int error;
    int haveData;

    // Pacote para dados temporários.
    AVPacket inputPacket;

    try
    {
        // inicia a FIFO
        objQueue = new Queue(codecContext, formatContext);
    }
    catch(BadAllocException)
    {
        throw;
    }
    catch(...)
    {
        throw GeneralException() << errno_code(MIR_DES_STMRADIO_1);
    }

int njn = 0;
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
                    // decodifica o frame
                    decodeAudioFrame(&haveData, &finished, &inputPacket);

                    /**
                    * Se o decodificador não terminar de processar os dados, esta função será chamada novamente.
                    */
                    if (finished && haveData)
                        finished = 0;

                    // libera memória do pacote temporário
                    av_free_packet(&inputPacket);

                    // Adiciona ao FIFO
                    objQueue->addQueueData(frame->data, frame->nb_samples);

                    av_frame_free(&frame);
//cout << "pacote " << njn++ << endl;
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

    statusConnection = MIR_CONNETION_CLOSING;
}

void StreamRadio::decodeAudioFrame(int *haveData, int *finished, AVPacket *inputPacket)
{
    int error=0;

    /**
     * Decodificar o frame de áudio armazenados no pacote temporário.
     * Se estamos no final do arquivo, passar um pacote vazio para o descodificador
     * para nivelá-lo.
     */
    if ((error = avcodec_decode_audio4(codecContext, frame,
                                       haveData, inputPacket)) < 0)
    {
        printf("Could not decode frame (error '%d')\n", error);
        throw DecoderException() << errno_code(MIR_ERR_DECODE);
    }
}

vector<vector<uint8_t>> StreamRadio::getQueueData()
{
    return objQueue->getQueueData();
}

int StreamRadio::getQueueSize()
{
    return objQueue->getQueueSize();
}

int StreamRadio::getChannelSize()
{
    return objQueue->getChannelSize();
}
