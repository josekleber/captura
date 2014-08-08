#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <boost/thread/thread.hpp>

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
#include <mir/tcpclient.h>

#include "exceptionmir.h"
#include "streamradio.h"
#include "database.h"

using namespace std;

struct ContextCreatorException : virtual BaseException {};
struct ConvertException : virtual BaseException {};

struct arqData_
{
    char arqName[26];
    uint8_t data[2048];
};

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

        AVFormatContext* getFormatContext(bool isRAW);
        AVCodecContext* getCodecContext(bool isRAW);
        AVCodecContext* getInCodecContext();
        SwrContext* getSwrContext(bool isRAW);

        // variaveis necessarias para geracao do fingerprint
        vector<Filter> *Filters;
        string ipRecognition;
        string portRecognition;
        string sqlConnString;
        string cutFolder;

        unsigned int* CreateFingerPrint(vector <uint8_t> Data, unsigned int* FingerPrintSize, bool mltFFT);

        void SetStreamRadio(unsigned int idxRadio, StreamRadio* oRadio);
        void CreateContext(string arqName, bool isRaw, AVDictionary* options);
        void ProcessFrames();
        void ProcessOutput();

        AVFormatContext* CreateFormatContext(string arqName, bool isRaw);
        AVCodecContext*  CreateCodecContext(AVFormatContext* frmContext, int channel, int SampleRate, AVSampleFormat SampleFormat, AVDictionary** outOptions);
        SwrContext* CreateSwrContext(AVCodecContext *inCodecContext, AVCodecContext *outCodecContext);
    protected:
    private:
        StreamRadio* objRadio;

        unsigned int idRadio;

        bool isExit;

        vector <unsigned char> bufRaw;
        AVFormatContext *rawFormatContext, *m4aFormatContext;
        AVCodecContext *rawCodecContext, *m4aCodecContext, *inCodecContext;
        SwrContext *rawSwrContext, *m4aSwrContext;

        AVFrame *rawInFrame;
        AVFrame *rawOutFrame;
        AVPacket rawOutPacket;

        AVFrame *m4aInFrame;
        AVFrame *m4aOutFrame;
        AVPacket m4aOutPacket;

        int maxFrames;

        bool lockFifo;
        AVAudioFifo *arqFifo = NULL;
        int cntRawDayCut;
        int cntM4aDayCut;
        void initFIFO();
        void addSamplesFIFO(uint8_t **inputSamples, const int frameSize);
        int getFifoData(void **data, int nb_samples);
        int getFifoSize();

        int EncodeFrames(bool isRAW);

        string getDateTime();
        string getDate();
        string getTime();
        string getSaveCutDir();
};

#endif // PARSER_H
