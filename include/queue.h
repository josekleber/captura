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

#include <mir/exceptionclass.h>

#include "util.h"


using namespace std;

class Queue
{
    public:
        Queue(AVCodecContext* codecContext, AVFormatContext* formatContext, int idRadio);
        virtual ~Queue();

        /** \brief
        * Retorna o tamanho da queue
        */
        int getQueueSize();     // quantidade de frames
        int getNbBuffers();     // quantidade de buffers
        int getSzBuffer();      // bytes por buffer

        /** \brief
        * Adiciona frame decodificado para a queue
        *
        * \param inputSamples   - dados decodificados que serão inseridos na queue
        * \param frameSize      - tamanho dos dados.
        */
        void addQueueData(AVFrame* frame);

        /** \brief
        * Retorna dados da queue, correspondente a um pacote
        */
//        int getQueueData(uint8_t **dest, int nbSamples);
        vector<vector<uint8_t>> getQueueData();
        void delQueueData();
    protected:
    private:
        boost::mutex mtx_;  // controle de lock da thread
        bool mtx = false;

        int idRadio;

        bool isPlannar;
        int szFrame;
        int nbBuffers;
        int szBuffers;
        int szData;
        int szBuffer;
        int nbChannels;
        enum AVSampleFormat Format;

        queue <vector<vector<uint8_t>>> queueData;

        // vetores auxiliares
        vector<vector <uint8_t>> buf1;
        vector <uint8_t> buf2;
};

#endif // QUEUE_H
