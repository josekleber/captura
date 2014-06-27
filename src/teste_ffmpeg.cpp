#include <iostream>
#include <cstdio>
#include <unistd.h>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/frame.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/error.h>
    #include <libswresample/swresample.h>
    #include <libavutil/avassert.h>
    #include <libavutil/audio_fifo.h>
}

#include "testes.h"

#define NELSON

#ifdef NELSON
#include "streamradio.h"
#include "parser.h"

int Testes::ffmpeg_teste(string arqNameIn, string arqNameOut)
{
    int ret;
    int stream_index;
    int got_frame;
    int data_present;

    AVFormatContext *outFormatContext;
    AVCodecContext  *outCodecContext, *inCodecContext;
    SwrContext* swrContext;

    AVFrame *inFrame = NULL, *outFrame = NULL;
    AVPacket inPacket, outPacket;

    AVAudioFifo* audioFifo = NULL;

    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    StreamRadio* objRadio = new StreamRadio;
    Parser* objParser = new Parser;

    objRadio->open(arqNameIn);

    inCodecContext = objRadio->getCodecContext();
    objRadio->read();
    audioFifo = *objRadio->getFIFO();

    objParser->SetInCodecContext(inCodecContext);
    objParser->CreateRAWContext(arqNameOut);
    outFormatContext = objParser->getFormatContext();
    outCodecContext = objParser->getCodecContext();
    swrContext = objParser->getSwrContext();

    av_dump_format(outFormatContext, 0, arqNameOut.c_str(), 1);

    inFrame = av_frame_alloc();
    outFrame = av_frame_alloc();

    int szFifo;
    int finished = 0;

    while(finished != 1)
    {
        const int outFrameSize = outCodecContext->frame_size;

        //uint8_t **inConvertedBuffer converted_input_samples = NULL;

        av_init_packet(&inPacket);

        szFifo = av_audio_fifo_size(audioFifo);


        int inFrameSize = inCodecContext->frame_size;
        int data_present = 0;

        while((szFifo = av_audio_fifo_size(audioFifo)) >= inFrameSize)
        {
            inFrame->nb_samples     = FFMIN(av_audio_fifo_size(audioFifo), inFrameSize);
            inFrame->channel_layout = inCodecContext->channel_layout;
            inFrame->format         = inCodecContext->sample_fmt;
            inFrame->sample_rate    = inCodecContext->sample_rate;

            outFrame->nb_samples     = outCodecContext->frame_size;
            outFrame->channel_layout = outCodecContext->channel_layout;
            outFrame->format         = outCodecContext->sample_fmt;
            outFrame->sample_rate    = outCodecContext->sample_rate;

            av_frame_get_buffer(inFrame, 0);
            av_frame_get_buffer(outFrame, 0);

            av_audio_fifo_read(audioFifo, (void**)&inFrame->data, inFrame->nb_samples);
            szFifo = av_audio_fifo_size(audioFifo);

            swr_convert(swrContext, (uint8_t**)&outFrame->data, outFrame->nb_samples, (const uint8_t**)inFrame->data, inFrame->nb_samples);

            av_init_packet(&outPacket);
            outPacket.data = NULL;
            outPacket.size = 0;

            ret = avcodec_encode_audio2(outCodecContext, &outPacket, outFrame, &data_present);

            if (data_present)
            {
                av_write_frame(outFormatContext, &outPacket);
            }

            av_free_packet(&outPacket);
        }
    }

    av_write_trailer(outFormatContext);
    avio_close(outFormatContext->pb);
}
#endif // NELSON




#ifdef KLEBER
struct buffer_data
{
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

using namespace std;

/* Duração da captura */
#define STREAM_DURATION     5.0

/* Bitrate de saída 64 Kbps */
#define STREAM_BITRATE      64000

/* Samplerate de saída 44.1 KHz */
#define STREAM_SAMPLERATE   44100

/* Canais de áudio de saída */
#define STREAM_CHANNEL      2

/* Sample format de saída */
#define STREAM_SAMPLEFMT    AV_SAMPLE_FMT_FLTP

static AVStream *add_audio_stream(AVFormatContext *fmt_ctx, AVCodecID codec_id)
{
    AVCodecContext *cdc_ctx;
    AVStream *st;
    st = avformat_new_stream(fmt_ctx, fmt_ctx->audio_codec);
    if (!st)
    {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    cdc_ctx = st->codec;
    cdc_ctx->codec_id = codec_id;
    cdc_ctx->codec_type = AVMEDIA_TYPE_AUDIO;

    /* put sample parameters */
    cdc_ctx->sample_fmt = STREAM_SAMPLEFMT;
    cdc_ctx->bit_rate = STREAM_BITRATE ;
    cdc_ctx->sample_rate = STREAM_SAMPLERATE;
    cdc_ctx->channels = STREAM_CHANNEL;
    cdc_ctx->channel_layout = AV_CH_LAYOUT_STEREO;


    // some formats want stream headers to be separate
    if(fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        cdc_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

/*
static void write_audio_frame(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;
    AVPacket pkt;
    av_init_packet(&pkt);

    c = st->codec;

    //get_audio_frame(samples, audio_input_frame_size, c->channels);

    pkt.size= avcodec_encode_audio(c, audio_outbuf, audio_outbuf_size, samples);

    if (c->coded_frame && c->coded_frame->pts != AV_NOPTS_VALUE)
        pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
    pkt.flags |= AV_PKT_FLAG_KEY;
    pkt.stream_index= st->index;
    pkt.data= audio_outbuf;

    // write the compressed frame in the media file
    if (av_interleaved_write_frame(oc, &pkt) != 0)
    {
        fprintf(stderr, "Error while writing audio frame\n");
        exit(1);
    }
}
*/

/**
 * Initialize the audio resampler based on the input and output codec settings.
 * If the input and output sample formats differ, a conversion is required
 * libswresample takes care of this, but requires initialization.
 */
static int init_resampler(AVCodecContext *input_codec_context,
                          AVCodecContext *output_codec_context,
                          SwrContext **resample_context)
{
    int error;

    /**
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity (they are sometimes not detected
     * properly by the demuxer and/or decoder).
     */
    *resample_context = swr_alloc_set_opts(NULL,
                                           av_get_default_channel_layout(output_codec_context->channels),
                                           output_codec_context->sample_fmt,
                                           output_codec_context->sample_rate,
                                           av_get_default_channel_layout(input_codec_context->channels),
                                           input_codec_context->sample_fmt,
                                           input_codec_context->sample_rate,
                                           0, NULL);
    if (!*resample_context)
    {
        fprintf(stderr, "Could not allocate resample context\n");
        return AVERROR(ENOMEM);
    }
    /**
    * Perform a sanity check so that the number of converted samples is
    * not greater than the number of samples to be converted.
    * If the sample rates differ, this case has to be handled differently
    */
    //av_assert0(output_codec_context->sample_rate == input_codec_context->sample_rate);

    /** Open the resampler with the specified parameters. */
    if ((error = swr_init(*resample_context)) < 0)
    {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(resample_context);
        return error;
    }
    return 0;
}

static int initFIFO(AVAudioFifo **fifo)
{
    /** Create the FIFO buffer based on the specified output sample format. */
    if (!(*fifo = av_audio_fifo_alloc(STREAM_SAMPLEFMT, STREAM_CHANNEL, 1)))
    {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

/** Initialize one data packet for reading or writing. */
static void init_packet(AVPacket *packet)
{
    av_init_packet(packet);
    /** Set the packet data and size so that it is recognized as being empty. */
    packet->data = NULL;
    packet->size = 0;
}

static char *const get_error_text(const int error)
{
    static char error_buffer[255];
    av_strerror(error, error_buffer, sizeof(error_buffer));
    return error_buffer;
}

/** Decode one audio frame from the input file. */
static int decode_audio_frame(AVFrame *frame,
                              AVFormatContext *input_format_context,
                              AVCodecContext *input_codec_context,
                              int *data_present, int *finished)
{
    /** Packet used for temporary storage. */
    AVPacket input_packet;
    int error;
    init_packet(&input_packet);

    /** Read one audio frame from the input file into a temporary packet. */
    if ((error = av_read_frame(input_format_context, &input_packet)) < 0)
    {
        /** If we are the the end of the file, flush the decoder below. */
        if (error == AVERROR_EOF)
            *finished = 1;
        else
        {
            fprintf(stderr, "Could not read frame (error '%s')\n",
                    get_error_text(error));
            return error;
        }
    }

    /**
     * Decodificar o frame de áudio armazenados no pacote temporário.
         * Se estamos no final do arquivo, passar um pacote vazio para o descodificador
     * para nivelá-lo.
     */
    if ((error = avcodec_decode_audio4(input_codec_context, frame,
                                       data_present, &input_packet)) < 0)
    {
        printf("Could not decode frame (error '%s')\n", get_error_text(error));
        av_free_packet(&input_packet);
        return error;
    }

    /**
     * If the decoder has not been flushed completely, we are not finished,
     * so that this function has to be called again.
     */
    if (*finished && *data_present)
        *finished = 0;
    av_free_packet(&input_packet);
    return 0;
}

/**
 * Initialize a temporary storage for the specified number of audio samples.
 * The conversion requires temporary storage due to the different format.
 * The number of audio samples to be allocated is specified in frame_size.
 */
static int init_converted_samples(uint8_t ***converted_input_samples,
                                  AVCodecContext *output_codec_context,
                                  int frame_size)
{
    int error;

    /**
     * Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = (uint8_t**)calloc(output_codec_context->channels,
                                     sizeof(**converted_input_samples))))
    {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }

    /**
     * Allocate memory for the samples of all channels in one consecutive
     * block for convenience.
     */
    if ((error = av_samples_alloc(*converted_input_samples, NULL,
                                  output_codec_context->channels,
                                  frame_size,
                                  output_codec_context->sample_fmt, 0)) < 0)
    {
        fprintf(stderr,
                "Could not allocate converted input samples (error '%s')\n",
                get_error_text(error));
        av_freep(&(*converted_input_samples)[0]);
        free(*converted_input_samples);
        return error;
    }
    return 0;
}

/**
 * Convert the input audio samples into the output sample format.
 * The conversion happens on a per-frame basis, the size of which is specified
 * by frame_size.
 */
static int convert_samples(const uint8_t **input_data,
                           uint8_t **converted_data, const int frame_size,
                           SwrContext *resample_context)
{
    int error;

    /** Convert the samples using the resampler. */
    if ((error = swr_convert(resample_context,
                             converted_data, frame_size,
                             input_data    , frame_size)) < 0)
    {
        fprintf(stderr, "Could not convert input samples (error '%s')\n",
                get_error_text(error));
        return error;
    }

    return 0;
}

/** Add converted input audio samples to the FIFO buffer for later processing. */
static int add_samples_to_fifo(AVAudioFifo *fifo,
                               uint8_t **converted_input_samples,
                               const int frame_size)
{
    int error;

    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0)
    {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }

    /** Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples,
                            frame_size) < frame_size)
    {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}


/**
 * Read one audio frame from the input file, decodes, converts and stores
 * it in the FIFO buffer.
 */
static int read_decode_convert_and_store(AVAudioFifo *fifo,
        AVFormatContext *input_format_context,
        AVCodecContext *input_codec_context,
        AVCodecContext *output_codec_context,
        SwrContext *resampler_context,
        int *finished)
{
    /** Temporary storage of the input samples of the frame read from the file. */
    AVFrame *input_frame = NULL;
    /** Temporary storage for the converted input samples. */
    uint8_t **converted_input_samples = NULL;
    int data_present;
    int ret = AVERROR_EXIT;

    /** Initialize temporary storage for one input frame. */
    if (!(input_frame = av_frame_alloc()))
    {
        fprintf(stderr, "Could not allocate input frame\n");
        goto cleanup;
    }
    /** Decode one frame worth of audio samples. */
    if (decode_audio_frame(input_frame, input_format_context,
                           input_codec_context, &data_present, finished))
        goto cleanup;
    /**
     * If we are at the end of the file and there are no more samples
     * in the decoder which are delayed, we are actually finished.
     * This must not be treated as an error.
     */
    if (*finished && !data_present)
    {
        ret = 0;
        goto cleanup;
    }
    /** If there is decoded data, convert and store it */
    if (data_present)
    {
        /** Initialize the temporary storage for the converted input samples. */
        if (init_converted_samples(&converted_input_samples, output_codec_context,
                                   input_frame->nb_samples))
            goto cleanup;

        /**
         * Convert the input samples to the desired output sample format.
         * This requires a temporary storage provided by converted_input_samples.
         */
        if (convert_samples((const uint8_t**)input_frame->extended_data, converted_input_samples,
                            input_frame->nb_samples, resampler_context))
            goto cleanup;

        /** Add the converted input samples to the FIFO buffer for later processing. */
        if (add_samples_to_fifo(fifo, converted_input_samples,
                                input_frame->nb_samples))
            goto cleanup;
        ret = 0;
    }
    ret = 0;

cleanup:
    if (converted_input_samples)
    {
        av_freep(&converted_input_samples[0]);
        free(converted_input_samples);
    }
    av_frame_free(&input_frame);

    return ret;
}

/**
 * Initialize one input frame for writing to the output file.
 * The frame will be exactly frame_size samples large.
 */
static int init_output_frame(AVFrame **frame,
                             AVCodecContext *output_codec_context,
                             int frame_size)
{
    int error;

    /** Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc()))
    {
        fprintf(stderr, "Could not allocate output frame\n");
        return AVERROR_EXIT;
    }

    /**
     * Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity.
     */

    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format         = output_codec_context->sample_fmt;
    (*frame)->sample_rate    = output_codec_context->sample_rate;

    /**
     * Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified.
     */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0)
    {
        fprintf(stderr, "Could allocate output frame samples (error '%s')\n",
                get_error_text(error));
        av_frame_free(frame);
        return error;
    }

    return 0;
}

/** Encode one frame worth of audio to the output file. */
static int encode_audio_frame(AVFrame *frame,
                              AVFormatContext *output_format_context,
                              AVCodecContext *output_codec_context,
                              int *data_present)
{
    /** Packet used for temporary storage. */
    AVPacket output_packet;
    int error;
    init_packet(&output_packet);

    /**
     * Encode the audio frame and store it in the temporary packet.
     * The output audio stream encoder is used to do this.
     */
    if ((error = avcodec_encode_audio2(output_codec_context, &output_packet,
                                       frame, data_present)) < 0)
    {
        fprintf(stderr, "Could not encode frame (error '%s')\n",
                get_error_text(error));
        av_free_packet(&output_packet);
        return error;
    }

    /** Write one audio frame from the temporary packet to the output file. */
    if (*data_present)
    {
        if ((error = av_write_frame(output_format_context, &output_packet)) < 0)
        {
            fprintf(stderr, "Could not write frame (error '%s')\n",
                    get_error_text(error));
            av_free_packet(&output_packet);
            return error;
        }

        av_free_packet(&output_packet);
    }

    return 0;
}


/**
 * Load one audio frame from the FIFO buffer, encode and write it to the
 * output file.
 */
static int load_encode_and_write(AVAudioFifo *fifo,
                                 AVFormatContext *output_format_context,
                                 AVCodecContext *output_codec_context)
{
    /** Temporary storage of the output samples of the frame written to the file. */
    AVFrame *output_frame;
    /**
     * Use the maximum number of possible samples per frame.
     * If there is less than the maximum possible frame size in the FIFO
     * buffer use this number. Otherwise, use the maximum possible frame size
     */
    const int frame_size = FFMIN(av_audio_fifo_size(fifo),
                                 output_codec_context->frame_size);
    int data_written;

    /** Initialize temporary storage for one output frame. */
    if (init_output_frame(&output_frame, output_codec_context, frame_size))
        return AVERROR_EXIT;

    /**
     * Read as many samples from the FIFO buffer as required to fill the frame.
     * The samples are stored in the frame temporarily.
     */
    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size)
    {
        fprintf(stderr, "Could not read data from FIFO\n");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }

    /** Encode one frame worth of audio samples. */
    if (encode_audio_frame(output_frame, output_format_context,
                           output_codec_context, &data_written))
    {
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    av_frame_free(&output_frame);
    return 0;
}


int Testes::FFmpeg()
{
    AVFormatContext *fmt_ctx_in = NULL, *fmt_ctx_out = NULL;
    AVOutputFormat *out_fmt = NULL;
    AVStream *stm_in = NULL, *stm_out = NULL;
    AVCodecContext *cdc_ctx_in = NULL, *cdc_ctx_out = NULL;
    AVCodec *cdc_out = NULL;
    AVFrame *frame = NULL;
    AVIOContext *io_ctx = NULL;
    AVPacket pkt;
    SwrContext * swr_ctx = NULL;
    AVAudioFifo *fifo = NULL;
    int ret=0,got_frame;

    /* registrando compoentes do FFMPEG */
    av_register_all();
    avcodec_register_all();
    avformat_network_init();



    StreamRadio *streamInput = new StreamRadio();
    string uri_in = "mmsh://radio.tokhost.com.br/germaniafm";
    //string uri_in = "/home/kleber/git/captura/bin/Debug/entrada.aac";
    //string uri_in = "/home/kleber/projetos/mir/captura/bin/Debug/1.mp3";
    string uri_out = "/home/kleber/git/captura/bin/Debug/saida.qq";

    streamInput->open(&uri_in);

    streamInput->read();


    /*
        fmt_ctx_in = streamInput->open(&uri_in);
        stm_in = streamInput->getStream();
        cdc_ctx_in = streamInput->getCodecContext();

        EnumStatusConnect status = streamInput->getStatus();

        if ((status == MIR_CONNECTION_OPEN))
        {
            printf("conexão aberta\n");

            // OBS.: format_name = mpeg, mp3, wav, flac, ipod, asf
            avformat_alloc_output_context2(&fmt_ctx_out,NULL,"wav",uri_out.c_str());

            out_fmt = fmt_ctx_out->oformat;
            stm_out = add_audio_stream(fmt_ctx_out,out_fmt->audio_codec);

            // dump input information to stderr
            av_dump_format(fmt_ctx_in, 0, uri_in.c_str(), 0);
            av_dump_format(fmt_ctx_out,0,uri_out.c_str(),1);

            // abrindo contexto de saída
            avio_open(&io_ctx,uri_out.c_str(),AVIO_FLAG_WRITE);
            // atribuindo IO para o contexto
            fmt_ctx_out->pb = io_ctx;

            printf("nome do codec de saída %s\n",fmt_ctx_out->oformat->name);

            // pegando o codec de saída
            stm_out = fmt_ctx_out->streams[0];
            cdc_out = avcodec_find_encoder(stm_out->codec->codec_id);
            cdc_ctx_out = avcodec_alloc_context3(cdc_out);
            cdc_ctx_out = stm_out->codec;
            avcodec_open2(cdc_ctx_out,cdc_out,NULL);

            init_resampler(cdc_ctx_in,cdc_ctx_out,&swr_ctx);

            initFIFO(&fifo);

            avformat_write_header(fmt_ctx_out, NULL);

            printf("Demuxing audio from file '%s' into '%s'\n", uri_in.c_str(), uri_out.c_str());

            //int decoded;
            //int audio_frame_count=0;

            while (true)
            {
                const int output_frame_size = cdc_ctx_out->frame_size;
                int finished                = 0;

                // verifica se há buffer no FIFO
                while (av_audio_fifo_size(fifo) < output_frame_size)
                {
                    //
                    // Decode one frame worth of audio samples, convert it to the
                    // output sample format and put it into the FIFO buffer.
                    //
                    read_decode_convert_and_store(fifo, fmt_ctx_in,
                                                  cdc_ctx_in,
                                                  cdc_ctx_out,
                                                  swr_ctx, &finished);
                    if (finished)
                        break;

                }

                //
                // If we have enough samples for the encoder, we encode them.
                // At the end of the file, we pass the remaining samples to
                // the encoder.
                //
                while (av_audio_fifo_size(fifo) >= output_frame_size ||
                        (finished && av_audio_fifo_size(fifo) > 0))
                    //
                    // Take one frame worth of audio samples from the FIFO buffer,
                    // encode it and write it to the output file.
                    //
                    load_encode_and_write(fifo, fmt_ctx_out,
                                          cdc_ctx_out);


                /*
                        AVPacket *orig_pkt = &pkt;

                        av_init_packet(&pkt);
                        frame = av_frame_alloc();
                        if (av_read_frame(fmt_ctx_in, &pkt) < 0)
                            break;

                        do
                        {
                            decoded = pkt.size;
                            ret = 0;

                            ret = avcodec_decode_audio4(cdc_ctx_in,frame,&got_frame,&pkt);

                            decoded = FFMIN(ret, pkt.size);

                            if (got_frame)
                                printf("gente, o programa tá lendo... %d\n",audio_frame_count++);

                            av_frame_unref(frame);

                            if (decoded < 0)
                                break;
                            pkt.data += decoded;
                            pkt.size -= decoded;
                        }
                        while (pkt.size > 0);

                        av_free_packet(orig_pkt);
                */
    //av_free_packet(pkt);
    // }


    //fmt_ctx_in->


    //avformat_write_header(fmt_ctx_out,NULL);


//    }
//    else
//    {
//        printf("falha na abertura da conexão.\n");
//    }

    /*
    // verifica o status da conexão
    if ((status == MIR_CONNECTION_OPEN))
    {
        printf("conexão aberta\n");
        frame = av_frame_alloc();
        //av_free(frame);

        if ((cdc_ctx_in->codec == NULL))
            printf("Codec not found.\n");
        else
        {
            avcodec_open2(cdc_ctx_in,cdc_ctx_in->codec,NULL);

            std::cout << "This stream has " << cdc_ctx_in->channels << " channels and a sample rate of " << cdc_ctx_in->sample_rate << "Hz" << std::endl;
            std::cout << "The data is in the format " << av_get_sample_fmt_name(cdc_ctx_in->sample_fmt) << std::endl;

            AVPacket pkt;
            av_init_packet(&pkt);

            while (av_read_frame(fmt_ctx_in,&pkt) == 0)
            {
                if (pkt.stream_index == stm->index)
                {
                    AVPacket decodingPacket = pkt;

                    while (decodingPacket.size > 0)
                    {
                        int gotFrame = 0;
                        int result = avcodec_decode_audio4(cdc_ctx_in, frame, &gotFrame, &decodingPacket);
                        printf("aqui.\n");
                    }

                }
            }


        }
    }
    else
    {
        printf("falha na abertura da conexão.\n");
    }

    */

}
#endif // KLEBER
