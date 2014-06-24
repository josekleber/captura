#ifndef PARSER_H
#define PARSER_H

#include <vector>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/audio_fifo.h>
}

using namespace std;

class Parser
{
    public:
        /** Default constructor */
        Parser();
        /** Default destructor */
        virtual ~Parser();

        /** \brief rotina responsavel pela leitura dos dados armazenados no fifo
         * A rotina le os packets armazenados na fifo (strutura do ffmpeg) e devolve um vetor com os dados RAW. Se o vetor for NULL, indica erro ou sem dados.
         * Ele tambem armazena em um vetor interno, os dados para o arquivo de saida no formato M4A.
         * A rotina, sempre no inicio, inicializa os vetores, ou seja, uma vez chamado a funcao, perde-se os dados anteriores.
         *
         * param *fifo AVAudioFifo
         *
         * \return retorna o vetor com os dados RAW
         *
         */
        unsigned char* ReadData(AVAudioFifo *fifo);
        int WriteToArq();
    protected:
    private:
        vector <unsigned char> bufRaw;
        AVFormatContext *rawFormatContext, *m4aFormatContext;
        AVCodecContext *rawCodecContext, *m4aCodecContext;

};

#endif // PARSER_H
