#ifndef PARSER_H
#define PARSER_H

#include <vector>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avstring.h>
    #include <libavutil/avutil.h>
    #include <libavutil/audio_fifo.h>
    #include <libavutil/frame.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/error.h>
    #include <libavutil/avassert.h>
    #include <libswresample/swresample.h>
}

#include <mir/fingerprint.h>

#include "exceptionmir.h"
#include "streamradio.h"

using namespace std;

struct ContextCreatorException : virtual BaseException {};
struct ConvertException : virtual BaseException {};

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

        string getDateTime();

        AVFormatContext* getFormatContext(bool isRAW);
        AVCodecContext* getCodecContext(bool isRAW);
        AVCodecContext* getInCodecContext();
        SwrContext* getSwrContext(bool isRAW);

        unsigned int* CreateFingerPrint(vector<Filter> Filters, vector <uint8_t> Data, unsigned int* FingerPrintSize, pthread_mutex_t* MutexAccess, bool mltFFT);

        void SetInCodecContext(AVCodecContext* inContext);
        void CreateContext(string arqName, bool isRaw, AVDictionary* options);
        void ConvertFrames();

        AVFormatContext* CreateFormatContext(string arqName, bool isRaw);
        AVCodecContext*  CreateCodecContext(AVFormatContext* frmContext, int chanell, int SampleRate, AVSampleFormat SampleFormat, AVDictionary** outOptions);
        SwrContext* CreateSwrContext(AVCodecContext *inCodecContext, AVCodecContext *outCodecContext);
    protected:
    private:
        vector <unsigned char> bufRaw;
        AVFormatContext *rawFormatContext, *m4aFormatContext;
        AVCodecContext *rawCodecContext, *m4aCodecContext, *inCodecContext;
        SwrContext *rawSwrContext, *m4aSwrContext;
        AVAudioFifo *fifo;
};

#endif // PARSER_H
