#include <iostream>
#include <boost/exception/all.hpp>
#include "queuehandler.h"
#include "streamradio.h"

using namespace std;
using namespace AMQP;


struct exception_base: virtual std::exception, virtual boost:: exception {};

int main()
{
    cout << "Hello world!" << endl;

    AVFormatContext *fmt_ctx_in = NULL, *fmt_ctx_out = NULL;
    AVStream *stm = NULL;
    AVCodecContext *cdc_ctx_in = NULL;
    AVFrame *frame = NULL;
    AVIOContext *io_ctx = NULL;

    /* registrando compoentes do FFMPEG */
    av_register_all();
    avcodec_register_all();
    avformat_network_init();


    StreamRadio *streamInput = new StreamRadio();
    //string uri = "mmsh://radio.tokhost.com.br/germaniafm";
    string uri_in = "/home/kleber/projetos/mir/captura/bin/Debug/1.mp3";
    string uri_out = "/home/kleber/projetos/mir/captura/bin/Debug/saida.mp3";

    fmt_ctx_in = streamInput->open(&uri_in);
    stm = streamInput->getStream();
    cdc_ctx_in = streamInput->getCodecContext();

    EnumStatusConnect status = streamInput->getStatus();

    if ((status == MIR_CONNECTION_OPEN))
    {
        printf("conexão aberta\n");

        fmt_ctx_out = avformat_alloc_context();

        // abrindo contexto de saída
        avio_open(&io_ctx,uri_out.c_str(),AVIO_FLAG_WRITE);

        fmt_ctx_out->pb = io_ctx;

        fmt_ctx_out->oformat = av_guess_format(NULL,uri_out.c_str(),NULL);



    }
    else
    {
        printf("falha na abertura da conexão.\n");
    }

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

    return 0;
}
