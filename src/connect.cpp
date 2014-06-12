#include "connect.h"

Connect::Connect()
{
    // zera o timer
    timer = 0;
    formatContext = NULL;
    codecContext = NULL;
    stream = NULL;
    dictionary = NULL;
    statusConnection = MIR_CONNETION_CLOSE;
}

Connect::~Connect()
{
    //dtor
    if ((formatContext))
        avformat_free_context(formatContext);
}

double Connect::getConnectionTime()
{
    if ((statusConnection != MIR_CONNECTION_OPEN))
        return 0;

    clock_t now = clock();

    double elapsed_secs = double(now - timer) / CLOCKS_PER_SEC;

    return elapsed_secs;
}

EnumStatusConnect Connect::getStatus()
{
    return statusConnection;
}

void Connect::close()
{
    if ((formatContext))
        avformat_close_input(&formatContext);

    timer = 0;
    statusConnection = MIR_CONNETION_CLOSE;
}

AVFormatContext* Connect::open(std::string *uri)
{
    formatContext = avformat_alloc_context();
    int ret = 0; // retorno das funções FFMPEG

    // verifica se conseguiu alocar o contexto
    if ((formatContext))
        throw BadAllocException() << errno_code(MIR_ERR_BADALLOC_CONTEXT);

    // força RTSP usar TCP
    rtspDetect(uri);

    // abre a conexão
    if ((ret=avformat_open_input(&formatContext,uri->c_str(),NULL,&dictionary)))
    {
        statusConnection = MIR_CONNECTION_ERROR;
        throw OpenConnectionException() <<errno_code(MIR_ERR_STREAM_CONNECTION);
    }

    setStreamType(); // pega os streams que a conexão contém

    timer = clock(); // inicia a contagem do tempo de conexão

    statusConnection = MIR_CONNECTION_OPEN;

    return formatContext;
}

void Connect::setStreamType()
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
            }
        }
    }

    // se chegar aqui e não tiver o stream é sinal de que não existe na conexão o AVMEDIA_TYPE_AUDIO
    // neste caso lanço a exceção
    if (!(stream))
        throw MediaTypeNoAudioException() << errno_code(MIR_ERR_MEDIA_TYPE_NO_AUDIO);

}

void Connect::addOptions(std::string *key, std::string *value)
{
    ///TODO: adicionar o controle de FLAGS. falta estudo de sua aplicação
    av_dict_set(&dictionary,key->c_str(),value->c_str(),0);
}

AVDictionary * Connect::getListOptions()
{
    return dictionary;
}

StreamType * Connect::getStreamType()
{
    if ((statusConnection != MIR_CONNECTION_OPEN))
        throw ConnectionClosedException() <<errno_code(MIR_ERR_CONNECTION_CLOSED);

    return streamType;
}

AVCodecContext * Connect::getCodecContext()
{
    return codecContext;
}

AVStream * Connect::getStream()
{
    return stream;
}

void Connect::rtspDetect(std::string *uri)
{
    int pos = (int)(uri->find("rtsp://",0));

    if ((pos > 0))
    {
        av_dict_set(&dictionary,"rtsp_transport","tcp",0);
    }
}
