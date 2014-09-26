#ifndef QUEUE_H
#define QUEUE_H

#include <string>
#include <vector>
#include <queue>

extern "C"
{
    /** inclusão dos headers FFMPEG */
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/audio_fifo.h>
}

#include <boost/thread.hpp>

#include "util.h"
#include "exceptionmir.h"

#define FIFO

using namespace std;

class Queue
{
    public:
        Queue(AVCodecContext* codecContext, AVFormatContext* formatContext);
        virtual ~Queue();

        /** \brief
        * Retorna o tamanho da queue
        */
        int getQueueSize();

        /** \brief
        * Adiciona frame decodificado para a queue
        *
        * \param inputSamples   - dados decodificados que serão inseridos na queue
        * \param frameSize      - tamanho dos dados.
        */
        void addQueueData(uint8_t **inputSamples, const int frameSize);

        /** \brief
        * Retorna dados da queue, correspondente a um pacote
        */
//        int getQueueData(uint8_t **dest, int nbSamples);
        vector<vector<uint8_t>> getQueueData();
        int getChannelSize();
    protected:
    private:
        boost::mutex mtx_;  // controle de lock da thread
        bool mtx = false;
        queue <vector<vector<uint8_t>>> queueData;
        int SampleSize;
        int FrameSize;
        int nbChannel;
        int bufSize;
        int nbBuffers;

#ifdef FIFO
        AVAudioFifo *fifo = NULL;
#endif // FIFO
        void initFIFO(AVCodecContext* codecContext, AVFormatContext* formatContext);
};

#endif // QUEUE_H
