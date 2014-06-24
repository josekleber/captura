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
    statusConnection = MIR_CONNETION_CLOSE;
}

StreamRadio::~StreamRadio()
{
    //dtor
    if ((formatContext))
        avformat_free_context(formatContext);

    //TODO : matar todos os objetos de conexão
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

AVFormatContext* StreamRadio::open(string *uri)
{
    formatContext = avformat_alloc_context();
    int ret = 0; // retorno das funções FFMPEG

    // verifica se conseguiu alocar o contexto
    if (!(formatContext))
        throw BadAllocException() << errno_code(MIR_ERR_BADALLOC_CONTEXT);

    // força RTSP usar TCP
    rtspDetect(uri);

    // abre a conexão
    if ((ret=avformat_open_input(&formatContext,uri->c_str(),NULL,&dictionary))< 0)
    {
        statusConnection = MIR_CONNECTION_ERROR;
        throw OpenConnectionException() <<errno_code(MIR_ERR_STREAM_CONNECTION);
    }

    // pega informações do stream
    if ((ret=avformat_find_stream_info(formatContext,NULL)) < 0)
    {
        statusConnection = MIR_CONNECTION_ERROR;
        throw OpenConnectionException() <<errno_code(MIR_ERR_STREAM_CONNECTION);
    }

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

    streamType = new StreamType[formatContext->nb_streams];

    for (unsigned int index = 0; index < formatContext->nb_streams; index++)
    {
        streamType[index].id = index;
        streamType[index].streamType = formatContext->streams[index]->codec->codec_type;

        // mesmo que haja mais de um stream de audio, neste momento, considero apenas o primeiro.
        if (!(stream))
        {
            if ((streamType[index].streamType = AVMEDIA_TYPE_AUDIO))
            {
                stream = formatContext->streams[index];

                codec = avcodec_find_decoder(stream->codec->codec_id);

                printf("codec de entrada %s\n",codec->name);

                /// TODO verificar a necessidade desta função. ver avcodec_open2
                codecContext = avcodec_alloc_context3(codec);

                codecContext = stream->codec;

                if (avcodec_open2(codecContext,codec,NULL) < 0)
                    throw BadAllocException() <<errno_code(MIR_ERR_BADALLOC_CONTEXT);

                if ((codecContext->codec == NULL))
                    throw CodecNotSupportedException() <<errno_code(MIR_ERR_CODEC_NOT_SUPPORTED);
            }
        }
    }

    // se chegar aqui e não tiver o stream é sinal de que não existe na conexão o AVMEDIA_TYPE_AUDIO
    // neste caso lanço a exceção
    if (!(stream))
        throw MediaTypeNoAudioException() << errno_code(MIR_ERR_MEDIA_TYPE_NO_AUDIO);

}

void StreamRadio::addOptions(string *key, string *value)
{
    ///TODO: adicionar o controle de FLAGS. falta estudo de sua aplicação
    av_dict_set(&dictionary,key->c_str(),value->c_str(),0);
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

void StreamRadio::rtspDetect(string *uri)
{
    int pos = (int)(uri->find("rtsp://",0));

    if ((pos > 0))
    {
        av_dict_set(&dictionary,"rtsp_transport","tcp",0);
    }
}

void StreamRadio::initFIFO(AVAudioFifo **fifo)
{
    /** Create the FIFO buffer based on the specified output sample format.
       nb_samples  initial allocation size, in samples */


    if (!(*fifo = av_audio_fifo_alloc(codecContext->sample_fmt, codecContext->channels,1)))
    {
        printf("FIFO não pode ser alocado.\n");
        throw BadAllocException() <<errno_code(AVERROR(ENOMEM));
    }
}

void StreamRadio::readFrame()
{
    bool isTrue = true;
    int *finished=0;
    int error;

    int data;

    // Pacote para dados temporários.
    AVPacket inputPacket;

    // inicia a FIFO
    initFIFO(&fifo);

    while (isTrue)
    {
        frame = NULL;
        av_init_packet(&inputPacket);
        // ainda não entendi o porquê setar os dados abaixo. Porém, em todos os
        // exemplos da ferramenta foi inicializado então...
        inputPacket.data = NULL;
        inputPacket.size = 0;

        if (!(frame = av_frame_alloc()))
            throw BadAllocException() <<errno_code(MIR_ERR_BADALLOC_CONTEXT);

        // Lê um frame do áudio e coloca no pacote temporário.
        if ((error = av_read_frame(formatContext, &inputPacket)) < 0)
        {
            // Se for final de arquivo ( em caso de arquivo ).
            if (error == AVERROR_EOF)
                *finished = 1;
            else
                throw FrameReadException() <<errno_code(MIR_ERR_FRAME_READ);
        }

        try
        {
            // decodifica o frame
            decodeAudioFrame(&data,finished,&inputPacket);

            /**
            * Se o decodificador não terminar de processar os dados, esta função será chamada novamente.
            */
            if (finished && data)
                *finished = 0;

            // libera memória do pacote temporário
            av_free_packet(&inputPacket);

        }
        catch(DecoderException ex)
        {
            av_free_packet(&inputPacket);
        }

        // Adiciona ao FIFO
        addSamplesFIFO(frame->extended_data, frame->nb_samples);

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


}

void StreamRadio::addSamplesFIFO(uint8_t **inputSamples, const int frameSize)
{
    int error;

    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frameSize)) < 0)
    {
        printf("Could not reallocate FIFO\n");
    }

    /** Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)inputSamples,
                            frameSize) < frameSize)
    {
        printf("Could not write data to FIFO\n");
    }
}

