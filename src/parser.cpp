#include "parser.h"

Parser::Parser()
{
    CreateContextRAW();
    CreateContextM4A("Teste");
}

Parser::~Parser()
{
    //dtor
}



static char *const get_error_text(const int error)
{
    static char error_buffer[255];
    av_strerror(error, error_buffer, sizeof(error_buffer));
    return error_buffer;
}





unsigned char* Parser::ReadData(AVAudioFifo *fifo)
{
    bufRaw.push_back((unsigned char)'a');

    return NULL;
}

void Parser::CreateContextRAW()
{
    int err;

    rawFormatContext = avformat_alloc_context();
    if (rawFormatContext == NULL)
        throw ContextCreatorException() << errno_code(MIR_ERR_CREATE_FORMAT_CONTEXT);
    if ((err = avio_open(&rawFormatContext->pb, "oi.wav", AVIO_FLAG_WRITE)) < 0)
    {
        fprintf(stderr, "%s\n", get_error_text(err));
        throw ContextCreatorException() << errno_code(MIR_ERR_OPEN_STREAM);
    }
    if ((rawFormatContext->oformat = av_guess_format(NULL, "oi.wav", NULL)) == NULL)
        throw ContextCreatorException() << errno_code(MIR_ERR_CREATE_FORMAT_CONTEXT);
    //av_strlcpy(rawFormatContext->filename, arqNameOut.c_str(), sizeof(rawFormatContext->filename));

    AVCodec* auxCodec = avcodec_find_encoder(rawFormatContext->oformat->audio_codec);
    AVStream* auxStream = avformat_new_stream(rawFormatContext, auxCodec);


    rawCodecContext = auxStream->codec;
    rawCodecContext->channels       = 1;
    rawCodecContext->channel_layout = av_get_default_channel_layout(rawCodecContext->channels);
    rawCodecContext->sample_rate    = 44100;
    rawCodecContext->sample_fmt     = AV_SAMPLE_FMT_S16;
    rawCodecContext->bit_rate       = 1411200;

    if(rawFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        rawCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;

    if ((err = avcodec_open2(rawCodecContext, auxCodec, NULL)) < 0)
    {
        fprintf(stderr, "%s\n", get_error_text(err));
        throw ContextCreatorException() << errno_code(MIR_ERR_OPEN_CODEC);
    }


    rawSwrContext = swr_alloc_set_opts(NULL,
                       av_get_default_channel_layout(rawCodecContext->channels),
                       rawCodecContext->sample_fmt,
                       rawCodecContext->sample_rate,
                       av_get_default_channel_layout(inCodecContext->channels),
                       inCodecContext->sample_fmt,
                       inCodecContext->sample_rate,
                       0, NULL);
    if (!rawSwrContext)
        throw ContextCreatorException() << errno_code(MIR_ERR_ALLOC_SWR_CONTEXT);
    //av_assert0(output_codec_context->sample_rate == inCodecContext->sample_rate);

    /** Open the resampler with the specified parameters. */
    if ((err = swr_init(rawSwrContext)) < 0)
    {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&rawSwrContext);
        throw ContextCreatorException() << errno_code(MIR_ERR_ALLOC_SWR_CONTEXT);
    }
}

void Parser::CreateContextM4A(string arqName)
{
    int err;

    m4aFormatContext = avformat_alloc_context();
    if (m4aFormatContext == NULL)
        throw ContextCreatorException() << errno_code(MIR_ERR_CREATE_FORMAT_CONTEXT);
    if ((err = avio_open(&m4aFormatContext->pb, (arqName + ".m4a").c_str(), AVIO_FLAG_WRITE)) < 0)
    {
        fprintf(stderr, "%s\n", get_error_text(err));
        throw ContextCreatorException() << errno_code(MIR_ERR_OPEN_STREAM);
    }
    if ((m4aFormatContext->oformat = av_guess_format(NULL, (arqName + ".m4a").c_str(), NULL)) == NULL)
        throw ContextCreatorException() << errno_code(MIR_ERR_CREATE_FORMAT_CONTEXT);
    av_strlcpy(m4aFormatContext->filename, (arqName + ".m4a").c_str(), sizeof(m4aFormatContext->filename));

    AVCodec* auxCodec = avcodec_find_encoder(m4aFormatContext->oformat->audio_codec);
    AVStream* auxStream = avformat_new_stream(m4aFormatContext, auxCodec);

    m4aCodecContext = auxStream->codec;
    m4aCodecContext->channels       = 1;
    m4aCodecContext->channel_layout = av_get_default_channel_layout(m4aCodecContext->channels);
    m4aCodecContext->sample_rate    = 44100;
    m4aCodecContext->sample_fmt     = AV_SAMPLE_FMT_S16;
    m4aCodecContext->bit_rate       = 1411200;

    if(m4aFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        m4aCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;

    if ((err = avcodec_open2(m4aCodecContext, auxCodec, NULL)) < 0)
    {
        fprintf(stderr, "%s\n", get_error_text(err));
        throw ContextCreatorException() << errno_code(MIR_ERR_OPEN_CODEC);
    }

    m4aSwrContext = swr_alloc_set_opts(NULL,
                       av_get_default_channel_layout(m4aCodecContext->channels),
                       m4aCodecContext->sample_fmt,
                       m4aCodecContext->sample_rate,
                       av_get_default_channel_layout(inCodecContext->channels),
                       inCodecContext->sample_fmt,
                       inCodecContext->sample_rate,
                       0, NULL);
    if (!m4aSwrContext)
        throw ContextCreatorException() << errno_code(MIR_ERR_ALLOC_SWR_CONTEXT);
    //av_assert0(output_codec_context->sample_rate == inCodecContext->sample_rate);

    /** Open the resampler with the specified parameters. */
    if ((err = swr_init(m4aSwrContext)) < 0)
    {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&m4aSwrContext);
        throw ContextCreatorException() << errno_code(MIR_ERR_ALLOC_SWR_CONTEXT);
    }
}


void Parser::ConvertFrame()
{
    int err;

    /** Temporary storage of the input samples of the frame read from the file. */
    AVFrame *inFrame = NULL, *outFrame = NULL;
    /** Temporary storage for the converted input samples. */
    uint8_t **convInSamples = NULL;
    int data_present;
    int ret = AVERROR_EXIT;

    int frame_size = FFMIN(av_audio_fifo_size(fifo), inCodecContext->frame_size);

    /** Initialize temporary storage for one input frame. */
    if (!(inFrame = av_frame_alloc()))
    {
        fprintf(stderr, "Could not allocate input frame\n");
        throw ConvertException() << errno_code(MIR_ERR_FRAME_ALLOC);
    }

    /**
     * Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity.
     */

    inFrame->nb_samples     = frame_size;
    inFrame->channel_layout = inCodecContext->channel_layout;
    inFrame->format         = inCodecContext->sample_fmt;
    inFrame->sample_rate    = inCodecContext->sample_rate;

    /**
     * Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified.
     */
    if ((err = av_frame_get_buffer(inFrame, 0)) < 0)
    {
        fprintf(stderr, "Could allocate output frame samples (error '%s')\n", get_error_text(err));
        av_frame_free(&inFrame);
        throw ConvertException() << errno_code(MIR_ERR_INIT_INPUT_FRAME);
    }

    if (av_audio_fifo_read(fifo, (void **)inFrame->data, frame_size) < frame_size)
    {
        fprintf(stderr, "Could not read data from FIFO\n");
        av_frame_free(&inFrame);
        throw ConvertException() << errno_code(MIR_ERR_READ_INPUT_FRAME);
    }

    if (!(convInSamples = (uint8_t**)calloc(rawCodecContext->channels, sizeof(**convInSamples))))
    {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        throw ConvertException() << errno_code(MIR_ERR_ALLOC_CONVERTED);
    }

    /**
     * Allocate memory for the samples of all channels in one consecutive
     * block for convenience.
     */
    if ((err = av_samples_alloc(convInSamples, NULL,
                                  rawCodecContext->channels,
                                  frame_size,
                                  rawCodecContext->sample_fmt, 0)) < 0)
    {
        fprintf(stderr, "Could not allocate converted input samples (error '%s')\n", get_error_text(err));
        av_freep(&(*convInSamples)[0]);
        free(*convInSamples);
        throw ConvertException() << errno_code(MIR_ERR_ALLOC_CONVERTED);
    }

    if ((err = swr_convert(rawSwrContext, convInSamples, frame_size, (const uint8_t**)inFrame->data, frame_size)) < 0)
    {
        fprintf(stderr, "Could not convert input samples (error '%s')\n", get_error_text(err));
        throw ConvertException() << errno_code(MIR_ERR_CONVERTER);
    }

    if (!(outFrame = av_frame_alloc()))
    {
        fprintf(stderr, "Could not allocate output frame\n");
        throw ConvertException() << errno_code(MIR_ERR_CONVERTER);
    }

    /**
     * Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity.
     */

    outFrame->nb_samples     = rawCodecContext->frame_size;
    outFrame->channel_layout = rawCodecContext->channel_layout;
    outFrame->format         = rawCodecContext->sample_fmt;
    outFrame->sample_rate    = rawCodecContext->sample_rate;

    /**
     * Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified.
     */
    if ((err = av_frame_get_buffer(outFrame, 0)) < 0)
    {
        fprintf(stderr, "Could allocate output frame samples (error '%s')\n", get_error_text(err));
        av_frame_free(&outFrame);
        throw ConvertException() << errno_code(MIR_ERR_FRAME_ALLOC);
    }

    memcpy(outFrame->data, *convInSamples, frame_size);
    outFrame->nb_samples = frame_size;

    /** Packet used for temporary storage. */
    AVPacket outPacket;

    av_init_packet(&outPacket);
    /** Set the packet data and size so that it is recognized as being empty. */
    outPacket.data = NULL;
    outPacket.size = 0;

    /**
     * Encode the audio frame and store it in the temporary packet.
     * The output audio stream encoder is used to do this.
     */
    if ((err = avcodec_encode_audio2(rawCodecContext, &outPacket, outFrame, &data_present)) < 0)
    {
        fprintf(stderr, "Could not encode frame (error '%s')\n", get_error_text(err));
        av_free_packet(&outPacket);
        throw ConvertException() << errno_code(MIR_ERR_ENCONDING);
    }

    /** Write one audio frame from the temporary packet to the output file. */
    if (data_present)
    {
        if ((err = av_write_frame(rawFormatContext, &outPacket)) < 0)
        {
            fprintf(stderr, "Could not write frame (error '%s')\n", get_error_text(err));
            av_free_packet(&outPacket);
            throw ConvertException() << errno_code(MIR_ERR_WRITE_FRAME);
        }

        av_free_packet(&outPacket);
    }
}
