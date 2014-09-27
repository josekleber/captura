#include "queue.h"

Queue::Queue(AVCodecContext* codecContext, AVFormatContext* formatContext)
{
    this->nbChannels = codecContext->channels;
    this->Format = codecContext->sample_fmt;
    this->isPlannar = av_sample_fmt_is_planar(codecContext->sample_fmt);
    this->szData = av_get_bytes_per_sample(codecContext->sample_fmt);
    this->nbBuffers = av_sample_fmt_is_planar(codecContext->sample_fmt) ? codecContext->channels : 1;
    this->szFrame = codecContext->frame_size;
    szBuffers = av_samples_get_buffer_size(&this->szBuffer, codecContext->channels, codecContext->frame_size,
                                           codecContext->sample_fmt, 1);
}

Queue::~Queue()
{
    while(!queueData.empty())
        queueData.pop();
}

int Queue::getQueueSize()
{
    //boost::lock_guard<boost::mutex> guard(mtx_);
    while (mtx)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    mtx = true;
    try
    {
        int ret;
        ret = queueData.size() * szFrame;
        mtx = false;
        return ret;
    }
    catch(...)
    {
        mtx = false;
        throw FifoException() << errno_code(MIR_ERR_FIFO_SIZE);
    }
}

int Queue::getSzBuffer()
{
    return this->szBuffer;
}

int Queue::getNbBuffers()
{
    return this->nbBuffers;
}

void Queue::addQueueData(AVFrame* frame)
{
    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
//    boost::lock_guard<boost::mutex> guard(mtx_);
    while (mtx)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    mtx = true;

    if (frame->linesize[0] != szBuffer)
        throw FifoException() << errno_code(MIR_ERR_FIFO_BUFFER_SIZE);
    if (frame->format != Format)
        throw FifoException() << errno_code(MIR_ERR_FIFO_ADD_FORMAT);
    if (frame->channels != nbChannels)
        throw FifoException() << errno_code(MIR_ERR_FIFO_ADD_CHANNELS);
    try
    {
        for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
        {
            for (int data = 0; data < szBuffer; data++)
                buf2.push_back(frame->data[numBuf][data]);
            buf1.push_back(buf2);
            buf2.clear();
        }

        queueData.push(buf1);

        for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
            buf1[numBuf].clear();

        buf1.clear();
        mtx = false;
    }
    catch(...)
    {
        mtx = false;
        throw FifoException() << errno_code(MIR_ERR_FIFO_ADD);
    }
}

vector<vector<uint8_t>> Queue::getQueueData()
{
    vector<vector<uint8_t>> ret;
//    boost::lock_guard<boost::mutex> guard(mtx_);
    while (mtx)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    mtx = true;
    try
    {
        if (!queueData.empty())
        {
            ret = queueData.front();

            queueData.pop();
        }

        mtx = false;
        return ret;
    }
    catch(...)
    {
        mtx = false;
        throw FifoException() << errno_code(MIR_ERR_FIFO_GET);
    }
}
