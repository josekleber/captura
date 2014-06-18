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
}

StreamRadio::~StreamRadio()
{
    //dtor
    if ((formatContext))
        avformat_free_context(formatContext);
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
                codecContext = stream->codec;
                codecContext->codec = avcodec_find_decoder(codecContext->codec_id);

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
