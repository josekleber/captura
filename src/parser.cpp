#include "parser.h"

Parser::Parser()
{
}

Parser::~Parser()
{
    //dtor
}





// somente para teste
static char *const get_error_text(const int error)
{
    static char error_buffer[255];
    av_strerror(error, error_buffer, sizeof(error_buffer));
    return error_buffer;
}



AVFormatContext* Parser::getFormatContext()
{
    return rawFormatContext;
}

AVCodecContext* Parser::getCodecContext()
{
    return rawCodecContext;
}

AVCodecContext* Parser::getInCodecContext()
{
    return inCodecContext;
}

SwrContext* Parser::getSwrContext()
{
    return rawSwrContext;
}

unsigned char* Parser::ReadData(AVAudioFifo *fifo)
{
    bufRaw.push_back((unsigned char)'a');

    return NULL;
}

void Parser::CreateRAWContext(string arqName)
{
    AVDictionary* options = NULL;

    rawFormatContext = CreateFormatContext(arqName);
    rawCodecContext = CreateCodecContext(rawFormatContext, 1, 44100, AVSampleFormat::AV_SAMPLE_FMT_S16, 1411200, &options);
    rawSwrContext = CreateSwrContext(inCodecContext, rawCodecContext);
}

void Parser::CreateM4AContext(string arqName)
{
    AVDictionary* options = NULL;

    m4aFormatContext = CreateFormatContext(arqName);
    m4aCodecContext = CreateCodecContext(m4aFormatContext, 2, 44100, AVSampleFormat::AV_SAMPLE_FMT_S16, 640000, &options);
    m4aSwrContext = CreateSwrContext(inCodecContext, m4aCodecContext);
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

void Parser::SetInCodecContext(AVCodecContext* inContext)
{
    inCodecContext = inContext;
}

AVFormatContext* Parser::CreateFormatContext(string arqName)
{
    AVIOContext *ioContext = NULL;
    AVFormatContext *outFormatContext = NULL;

    if (avio_open(&ioContext, arqName.c_str(), AVIO_FLAG_WRITE) < 0)
        throw ConvertException() << errno_code(MIR_ERR_OPEN_OUTPUT_FILE);
    if (!(outFormatContext = avformat_alloc_context()))
        throw ConvertException() << errno_code(MIR_ERR_OPEN_FORMAT_CONTEXT);

    outFormatContext->pb = ioContext;

    if (!(outFormatContext->oformat = av_guess_format(NULL, arqName.c_str(), NULL)))
        throw ConvertException() << errno_code(MIR_ERR_OPEN_OUTPUT_FORMAT);

    int len;
    for (len = 0; (arqName.c_str())[len] != 0; outFormatContext->filename[len] = (arqName.c_str())[len++]);
    outFormatContext->filename[len] = 0;

    return outFormatContext;
}

AVCodecContext* Parser::CreateCodecContext(AVFormatContext* frmContext, int chanell, int SampleRate, AVSampleFormat SampleFormat, int BitRate, AVDictionary** outOptions)
{
    AVCodec *outCodec = NULL;
    AVStream *outStream = NULL;
    AVCodecContext* outCodecContext = NULL;

    if (!(outCodec = avcodec_find_encoder(frmContext->oformat->audio_codec)))
        throw ConvertException() << errno_code(MIR_ERR_OPEN_CODEC);
    if (!(outStream = avformat_new_stream(frmContext, outCodec)))
        throw ConvertException() << errno_code(MIR_ERR_OPEN_STREAM);
    outCodecContext = outStream->codec;

    outCodecContext->channels       = chanell;
    outCodecContext->channel_layout = av_get_default_channel_layout(chanell);
    outCodecContext->sample_rate    = SampleRate;
    outCodecContext->sample_fmt     = SampleFormat;
    outCodecContext->bit_rate       = BitRate;

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
