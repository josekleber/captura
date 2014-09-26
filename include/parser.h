#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <vector>
#include "util.h"

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
#include <libavutil/opt.h>
}

#include "exceptionmir.h"
#include "database.h"

using namespace std;

struct ContextCreatorException : virtual BaseException {};
struct ConvertException : virtual BaseException {};

class Parser
{
public:

    /** Default constructor */
    Parser();
    Parser(string fileName, uint64_t channelLayoutIn, int sampleRateIn, int bitRateIn,
           AVSampleFormat sampleFormatIn, int nbSamplesIn, int nbChannelIn);

    /** Default destructor */
    virtual ~Parser();

    /** \brief Enumerador do tipo do audio de saída */
    enum AUDIOFORMAT
    {
        raw=0,
        arq=1
    };

    vector<string> audioFormatList = {"wav","mp3"};

    void setBitRate(int value);
    void setSampleRate(int value);
    void setChannels(unsigned int value);
    void setBuffer(vector<vector<vector<uint8_t>>> value, int szChannel);

    /** \brief Processa os frames e gera a saída */
    virtual void Execute();

    /** \brief configura a execução do objeto Parser */
    void Config();


    string fileName;
    uint64_t channelLayoutIn;
    int sampleRateIn;
    int bitRateIn;
    AVSampleFormat sampleFormatIn;
    int nbSamplesIn;
    int nbChannelIn;


protected:
    int bitRate = 24000;
    int sampleRate = 11025;
    unsigned int nbChannel = 1;
    unsigned int nbFrames;
    bool isVBR;
    AUDIOFORMAT audioFormat;

    AVFormatContext *fmt_ctx_out;
    AVStream *stm_out = NULL;
    AVCodecContext *cdc_ctx_out = NULL;
    AVCodec *cdc_out = NULL;
    SwrContext *swr_ctx = NULL;
    AVDictionary *dic = NULL;
    AVFrame *frame_in = NULL;
    AVFrame *frame_out = NULL;
    AVPacket pkt_out;
    AVIOContext *io_ctx = NULL;

    vector<vector<vector<uint8_t>>> bufFrames;
    int szChannel;

    /** \brief efetua o resampling do audio, encodando, de acordo com o set do output */
    void Resample();

    virtual void EndResample();

    virtual void initObject();
private:
    /** \brief Cria contexto de saída */
    void CreateContext();

    /** \brief Define os parâmetros para o stream de saída */
    void setStream();

    /** \brief Inicializa o resample */
    void InitResampler();

    /** \brief Retorna o id do codec de acordo com o formato de saída */
    AVCodecID getCodecID();

    /** \brief Retorna o sample format de acordo com o formato de saída */
    AVSampleFormat getSampleFormat(AVCodecID codecID);

    void EndProcess();
};

#endif // PARSER_H
