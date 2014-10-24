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
}

StreamRadio::~StreamRadio()
{
    isExit = true;

    sleep(5);

    while (statusConnection == EnumStatusConnect::MIR_CONNECTION_OPEN)
        usleep(10);

    try
    {
        if (dictionary)
            av_free(dictionary);
        if (formatContext)
            avformat_free_context(formatContext);
        if (codecContext)
            avcodec_close(codecContext);
    }
    catch (SignalException& e)
    {
         objLog->mr_printf(MR_LOG_ERROR, idRadio, "Destructor StreamRadio: %s\n", e.what());
    }
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

AVFormatContext* StreamRadio::open(int idRadio, string uri)
{
    // retorno das funções FFMPEG
    int ret = 0;

    this->idRadio = idRadio;
    this->uri = uri;

    // verifica se conseguiu alocar o contexto
    formatContext = avformat_alloc_context();
    if (!(formatContext))
        throw StreamRadioException() << errno_code(MIR_ERR_BADALLOC_CONTEXT);

    // usa todos os cores do processador.
    addOptions("threads","0");

    // força RTSP usar TCP
    rtspDetect();

    // abre a conexão
    if ((ret=avformat_open_input(&formatContext, uri.c_str(), NULL, &dictionary))< 0)
    {
        char error_buffer[255];
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error (%d) : %s\n", ret, error_buffer);

        statusConnection = MIR_CONNECTION_ERROR;
        throw StreamRadioException() << errno_code(MIR_ERR_STREAM_CONNECTION1);
    }

    objLog->mr_printf(MR_LOG_MESSAGE, idRadio, MR_LOG_BOLDGREEN "Conectado ao stream %s" MR_LOG_RESET "\n", uri.c_str());

    statusConnection = MIR_CONNECTION_OPEN;

    try
    {
        setStreamType(); // pega os streams que a conexão contém
    }
    catch(SignalException& err)
    {
        throw ExceptionClass("streamradio", "open", "Erro de segmentacao na setagem do tipo de stream");
    }
    catch(...)
    {
        throw;
    }

    timer = clock(); // inicia a contagem do tempo de conexão

    av_dump_format(formatContext, streamIndex, uri.c_str(), 0);

    try
    {
        objQueue = new Queue(codecContext, formatContext, idRadio);
    }
    catch(SignalException& err)
    {
        throw ExceptionClass("streamradio", "open", "Erro de segmentacao na criacao do objQueue");
    }
    catch(FifoException& err)
    {
        throw;
    }
    catch(...)
    {
        throw GeneralException() << errno_code(MIR_DES_STMRADIO_1);
    }

    return formatContext;
}

void StreamRadio::read()
{
    if (statusConnection != MIR_CONNECTION_OPEN)
        throw StreamRadioException() << errno_code(MIR_ERR_CONNECTION_CLOSED);

    readFrame();
}

void StreamRadio::setStreamType()
{
    int ret;

    if ((statusConnection != MIR_CONNECTION_OPEN))
        throw StreamRadioException() <<errno_code(MIR_ERR_CONNECTION_CLOSED);

    // pega informações do stream
    if ((ret = avformat_find_stream_info(formatContext,NULL)) < 0)
    {
        char error_buffer[255];
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error (%d) : %s\n", ret, error_buffer);

        statusConnection = MIR_CONNECTION_ERROR;
        throw StreamRadioException() << errno_code(MIR_ERR_STREAM_CONNECTION1);
    }

    streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);

    if (streamIndex == AVERROR_STREAM_NOT_FOUND)
    {
        statusConnection = MIR_CONNECTION_ERROR;
        throw StreamRadioException() << errno_code(MIR_ERR_MEDIA_TYPE_NO_AUDIO);
    }
    else if (streamIndex == AVERROR_DECODER_NOT_FOUND)
    {
        statusConnection = MIR_CONNECTION_ERROR;
        throw StreamRadioException() <<errno_code(MIR_ERR_CODEC_NOT_SUPPORTED);
    }

    stream = formatContext->streams[streamIndex];
    codecContext = stream->codec;

    if ((ret = avcodec_open2(codecContext, codec, NULL)) < 0)
    {
        char error_buffer[255];
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error (%d) : %s\n", ret, error_buffer);

        statusConnection = MIR_CONNECTION_ERROR;
        throw StreamRadioException() << errno_code(MIR_ERR_BADALLOC_CONTEXT);
    }

    this->channelLayout = codecContext->channel_layout;

    // se chegar aqui e não tiver o stream é sinal de que não existe na conexão o AVMEDIA_TYPE_AUDIO
    // neste caso lanço a exceção
    if (!(stream))
    {
        statusConnection = MIR_CONNECTION_ERROR;
        throw StreamRadioException() << errno_code(MIR_ERR_MEDIA_TYPE_NO_AUDIO);
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
        throw StreamRadioException() <<errno_code(MIR_ERR_CONNECTION_CLOSED);

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
    int pos = this->uri.find("rtsp:", 0);

    if (pos >= 0)
    {
        av_dict_set(&dictionary, "rtsp_transport", "tcp", 0);
    }
}

void StreamRadio::readFrame()
{
    int finished = 0;
    int error;
    int haveData;

    // Pacote para dados temporários.
    AVPacket inputPacket;

int njn = 0;
    do
    {
        frame = av_frame_alloc();
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));  // jk
    } while (!frame);

    while ((finished == 0) && !isExit)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(15));  // jk
        av_init_packet(&inputPacket);
        // ainda não entendi o porquê setar os dados abaixo. Porém, em todos os
        // exemplos da ferramenta foi inicializado então...
        inputPacket.data = NULL;
        inputPacket.size = 0;

        try
        {
            // Lê um frame do áudio e coloca no pacote temporário.
            if ((error = av_read_frame(formatContext, &inputPacket)) < 0)
            {
                // Se for final de arquivo ( em caso de arquivo ).
                if (error == AVERROR_EOF)
                    finished = 1;
                else
                {
                    char error_buffer[255];
                    av_strerror(error, error_buffer, sizeof(error_buffer));
                    objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error (%d) : %s\n", error, error_buffer);

                    throw StreamRadioException() << errno_code(MIR_ERR_FRAME_READ);
                }
            }

            if (!finished)
            {
                try
                {
                    // decodifica o frame
                    decodeAudioFrame(&haveData, &inputPacket);

                    // libera memória do pacote temporário
                    av_free_packet(&inputPacket);
                }
                catch(StreamRadioException& err)
                {
                    av_free_packet(&inputPacket);
                    av_frame_unref(frame);
                    throw;
                }

                try
                {
                    // Adiciona ao FIFO
                    objQueue->addQueueData(frame);
                    av_frame_unref(frame);
//objLog->mr_printf(MR_LOG_DEBUG, idRadio, "Frame : %5d    Fifo : %d\n", njn++, objQueue->getQueueSize());

                    if (getQueueSize() > MAX_QUEUE_SIZE)
                        usleep(10);
                }
                catch(ExceptionClass& err)
                {
                    av_frame_unref(frame);
                    throw;
                }
                catch(FifoException& err)
                {
                    av_frame_unref(frame);
                    throw;
                }
            }
        }
        catch(SignalException& err)
        {
            objLog->mr_printf(MR_LOG_ERROR, idRadio, "streakradio (readFrame) : Erro de segmentacao\n");
        }
        catch(ExceptionClass& err)
        {
            objLog->mr_printf(MR_LOG_ERROR, idRadio, "%s\n", err.what());
        }
        catch(StreamRadioException& err)
        {
            objLog->mr_printf(MR_LOG_ERROR, idRadio, "Stream Error %d\n", *boost::get_error_info<errno_code>(err));
        }
        catch(FifoException& err)
        {
            objLog->mr_printf(MR_LOG_ERROR, idRadio, "Queue Error %d\n", *boost::get_error_info<errno_code>(err));
        }
        catch (...)
        {
            objLog->mr_printf(MR_LOG_ERROR, idRadio, "Ocorreu um erro no stream de entrada.\n");
        }
    }
    av_frame_free(&frame);

    statusConnection = MIR_CONNETION_CLOSING;
}

void StreamRadio::decodeAudioFrame(int *haveData, AVPacket *inputPacket)
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
        char error_buffer[255];
        av_strerror(error, error_buffer, sizeof(error_buffer));
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error (%d) : %s\n", error, error_buffer);

        throw StreamRadioException() << errno_code(MIR_ERR_DECODE);
    }
}

vector<vector<uint8_t>> StreamRadio::getQueueData()
{
    try
    {
        return objQueue->getQueueData();
    }
    catch(SignalException& err)
    {
        throw ExceptionClass("streamradio", "getQueueData", "Erro de segmentacao");
    }
    catch(FifoException& err)
    {
        throw;
    }
    catch(...)
    {
        throw;
    }
}

int StreamRadio::getQueueSize()
{
    try
    {
        return objQueue->getQueueSize();
    }
    catch(SignalException& err)
    {
        throw ExceptionClass("streamradio", "getQueueData", "Erro de segmentacao");
    }
    catch(FifoException& err)
    {
        throw;
    }
    catch(...)
    {
        throw;
    }
}

int StreamRadio::getSzBuffer()
{
    return objQueue->getSzBuffer();
}

void StreamRadio::delQueueData()
{
    objQueue->delQueueData();
}
