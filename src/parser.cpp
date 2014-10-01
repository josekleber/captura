#include "parser.h"
Parser::Parser()
{
}

Parser::~Parser()
{
    // para dar tempo de terminar algum processo pendente
    sleep(2);
//    boost::this_thread::sleep(boost::posix_time::seconds(2));

    EndProcess();
}

void Parser::EndProcess()
{
    try
    {
        if (dic)
            av_free(dic);
        if (io_ctx)
            avio_close(io_ctx);
        if (swr_ctx)
            swr_free(&swr_ctx);
        if (fmt_ctx_out)
            avformat_free_context(fmt_ctx_out);
        if (cdc_ctx_out)
            avcodec_close(cdc_ctx_out);
    }
    catch (SignalException& e)
    {
         objLog->mr_printf(MR_LOG_ERROR, idRadio, "Destructor Parser: %s\n", e.what());
    }
}

void Parser::Config()
{
    try
    {
        // cria o contexto de saída
        CreateContext();

        // seta os dados do encoder
        setStream();

        // inicia o resample
        InitResampler();

        nbBuffers = av_sample_fmt_is_planar(sampleFormatIn) ? this->nbChannelIn : 1;
    }
    catch(...)
    {
        throw;
    }
}

void Parser::CreateContext()
{
    int error = 0;

    error = avformat_alloc_output_context2(&fmt_ctx_out, NULL,
                        audioFormatList[audioFormat].c_str(), fileName.c_str());

    if (error < 0)
        throw ContextCreatorException() << errno_code(MIR_ERR_BADALLOC_CONTEXT);

    if(audioFormat == AUDIOFORMAT::raw)
        fmt_ctx_out->oformat->flags |= AVFMT_NOFILE;
    else
    {
        error = avio_open(&io_ctx, fileName.c_str(), AVIO_FLAG_WRITE);
        if (error < 0)
        {
            char error_buffer[255];
            av_strerror(error, error_buffer, sizeof(error_buffer));
            objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error: %s\n", error_buffer);
            throw ConvertException() << errno_code(MIR_ERR_OPEN_OUTPUT_FILE);
        }
    }

    fmt_ctx_out->pb = io_ctx;
}

void Parser::setStream()
{
    int error = 0;

    AVCodec *codec = NULL;
    AVCodecID codecID = getCodecID();

    codec = avcodec_find_encoder(codecID);
    fmt_ctx_out->audio_codec = codec;
    stm_out = avformat_new_stream(fmt_ctx_out, codec);

    if (!stm_out)
        throw StreamException() << errno_code(MIR_ERR_OPEN_STREAM);

    cdc_ctx_out = avcodec_alloc_context3(codec);

    cdc_ctx_out = stm_out->codec;
    cdc_ctx_out->codec = codec;
    cdc_ctx_out->codec_id = codecID;
    cdc_ctx_out->codec_type = AVMEDIA_TYPE_AUDIO;

    cdc_ctx_out->sample_fmt = getSampleFormat(codecID);

    if (!isVBR)
        cdc_ctx_out->bit_rate = bitRate;
    else
    {
        cdc_ctx_out->rc_max_rate = 0;
        cdc_ctx_out->rc_min_rate = 0;
        cdc_ctx_out->bit_rate_tolerance = bitRate;
        cdc_ctx_out->bit_rate = bitRate;
    }

    cdc_ctx_out->sample_rate = sampleRate;
    cdc_ctx_out->channels = nbChannel;
    cdc_ctx_out->channel_layout = av_get_default_channel_layout(nbChannel);

    error = avcodec_open2(cdc_ctx_out,codec,NULL);

    if (error < 0)
        throw StreamException() << errno_code(MIR_ERR_OPEN_STREAM);

    if (codecID == AV_CODEC_ID_PCM_S16LE || codecID == AV_CODEC_ID_MP3)
        cdc_ctx_out->frame_size = av_rescale_rnd(nbSamplesIn,
                                                 cdc_ctx_out->sample_rate, sampleRateIn,AV_ROUND_UP);
    else if (codecID == AV_CODEC_ID_AAC)
    {
        cdc_ctx_out->profile = FF_PROFILE_AAC_LOW;
        cdc_ctx_out->frame_size = 1024;

        // some formats want stream headers to be separate
        if(fmt_ctx_out->oformat->flags & AVFMT_GLOBALHEADER)
            cdc_ctx_out->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    cdc_out = codec;

    if (audioFormat == AUDIOFORMAT::arq)
    {
        fmt_ctx_out->oformat->flags |= AVFMT_ALLOW_FLUSH;
        error = avformat_write_header(fmt_ctx_out, NULL);

        if (error < 0)
            throw StreamException() << errno_code(MIR_ERR_OPEN_STREAM);
    }

    av_dump_format(fmt_ctx_out, 0, fileName.c_str(), 1);
}

void Parser::InitResampler()
{
    swr_ctx = swr_alloc_set_opts(NULL,
                                 av_get_default_channel_layout(cdc_ctx_out->channels),
                                 cdc_ctx_out->sample_fmt,
                                 cdc_ctx_out->sample_rate,
                                 channelLayoutIn,
                                 sampleFormatIn,
                                 sampleRateIn,
                                 0,0);

    if(!swr_ctx)
        throw ContextCreatorException() << errno_code(MIR_ERR_ALLOC_SWR_CONTEXT);

    // set options
    av_opt_set_int(swr_ctx, "in_channel_layout",    channelLayoutIn, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate",       sampleRateIn, 0);
    av_opt_set_int(swr_ctx, "in_bit_rate",       bitRateIn, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", sampleFormatIn, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout",    cdc_ctx_out->channel_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate",       cdc_ctx_out->sample_rate, 0);
    av_opt_set_int(swr_ctx, "out_bit_rate",       cdc_ctx_out->bit_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", cdc_ctx_out->sample_fmt, 0);

    if (swr_init(swr_ctx) < 0)
        throw ContextCreatorException() << errno_code(MIR_ERR_INIT_SWR_CONTEXT);
}

void Parser::Resample()
{
    int got_frame;

    frame_out = av_frame_alloc();
    if (frame_out == NULL)
        throw ResampleException() << errno_code(MIR_ERR_FRAME_ALLOC);

    frame_out->nb_samples = cdc_ctx_out->frame_size;
    frame_out->format = cdc_ctx_out->sample_fmt;
    frame_out->sample_rate = sampleRate;
    frame_out->channel_layout = av_get_default_channel_layout(nbChannel);

    if (av_frame_get_buffer(frame_out, 1) < 0)
        throw ResampleException() << errno_code(MIR_ERR_BUFFER_ALLOC);

    frame_in = av_frame_alloc();
    if (frame_in == NULL)
        throw ResampleException() << errno_code(MIR_ERR_FRAME_ALLOC);

    frame_in->nb_samples = nbSamplesIn;
    frame_in->format = sampleFormatIn;
    frame_in->sample_rate = sampleRateIn;
    frame_in->channel_layout = channelLayoutIn;

    if (av_frame_get_buffer(frame_in, 1) < 0)
        throw ResampleException() << errno_code(MIR_ERR_BUFFER_ALLOC);

    for (int idxFrame = 0; idxFrame < (int)this->bufFrames.size(); idxFrame++)
    {
//        boost::this_thread::sleep(boost::posix_time::milliseconds(1));

        vector<vector<uint8_t>> aux = bufFrames[idxFrame];

        if (aux.size() != nbBuffers)
            throw FifoException() << errno_code(MIR_ERR_FIFO_DATA1);

        for (int i = 0; i < (int)aux.size(); i++)
        {
            if (aux[i].size() != szBuffer)
                throw FifoException() << errno_code(MIR_ERR_FIFO_DATA2);
            memcpy(frame_in->data[i], aux[i].data(), szBuffer);
        }
        for (int i = 0; i < (int)aux.size(); i++)
            aux[i].clear();
        aux.clear();

        if (swr_convert(swr_ctx, (uint8_t**)&frame_out->data, frame_out->nb_samples,
                         (const uint8_t**)&frame_in->data, frame_in->nb_samples) >= 0)
        {
            got_frame = 0;
            av_init_packet(&pkt_out);
            pkt_out.data = NULL;
            pkt_out.size = 0;

            if (avcodec_encode_audio2(cdc_ctx_out, &pkt_out, frame_out, &got_frame) >= 0)
                EndResample();
            else
                throw ResampleException() << errno_code(MIR_ERR_ENCODE);

            av_free_packet(&pkt_out);
//            av_frame_unref(frame_out);
        }
        else
            throw ResampleException() << errno_code(MIR_ERR_RESAMPLE);
    }

    for (int idxFrame = 0; idxFrame < (int)this->bufFrames.size(); idxFrame++)
    {
        for (int i = 0; i < (int)bufFrames[idxFrame].size(); i++)
            bufFrames[idxFrame][i].clear();
        bufFrames[idxFrame].clear();
    }
    bufFrames.clear();
/**/

    av_frame_free(&frame_in);
    av_frame_free(&frame_out);
}

AVCodecID Parser::getCodecID()
{
    int ret = AV_CODEC_ID_NONE;

    string format = audioFormatList[audioFormat];

    if (format.compare("mp3") == 0)
        ret = AV_CODEC_ID_MP3;
    else if (format.compare("wav") == 0)
        ret = AV_CODEC_ID_PCM_S16LE;
    else if (format.compare("adts") == 0)
        ret = AV_CODEC_ID_AAC;
    else if (format.compare("flv") == 0)
        ret = AV_CODEC_ID_AAC;

    return (AVCodecID)ret;
}

AVSampleFormat Parser::getSampleFormat(AVCodecID codecID)
{
    AVSampleFormat ret = AV_SAMPLE_FMT_NONE;
    switch (codecID)
    {
    case AV_CODEC_ID_MP3:
        ret = AV_SAMPLE_FMT_S16P;
        break;
    case AV_CODEC_ID_AAC:
        ret = AV_SAMPLE_FMT_S16;
        break;
    case AV_CODEC_ID_PCM_S16LE:
        ret = AV_SAMPLE_FMT_S16;
        break;
    default:
        break;
    }

    return ret;
}

void Parser::setBitRate(int value)
{
    bitRate = value;
}

void Parser::setSampleRate(int value)
{
    sampleRate = value;
}

void Parser::setChannels(unsigned int value)
{
    nbChannel = value;
}

void Parser::setBuffer(vector<vector<vector<uint8_t>>> value)
{
    bufFrames = value;
}

void Parser::Execute(){}

void Parser::EndResample(){}

void Parser::initObject(){}
