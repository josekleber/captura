#include "queue.h"

Queue::Queue(AVCodecContext* codecContext, AVFormatContext* formatContext)
{
    this->nbChannel = codecContext->channels;
    this->FrameSize = codecContext->frame_size;
    this->nbBuffers = av_sample_fmt_is_planar(codecContext->sample_fmt) ? this->nbChannel : 1;
    av_samples_get_buffer_size(&this->bufSize, this->nbChannel, 1, codecContext->sample_fmt, 1);
    this->SampleSize = bufSize;
}

Queue::~Queue()
{
    //dtor
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
        ret = queueData.size() * FrameSize;
        mtx = false;
        return ret;
    }
    catch(...)
    {
        mtx = false;
        throw FifoException() << errno_code(MIR_ERR_FIFO_SIZE);
    }
}

void Queue::addQueueData(uint8_t **inputSamples, const int frameSize)
{
    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
//    boost::lock_guard<boost::mutex> guard(mtx_);
    while (mtx)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    mtx = true;
    try
    {
        vector<vector <uint8_t>> buf1;
        for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
        {
            vector <uint8_t> buf2;
            for (int data = 0; data < frameSize; data++)
                for (int qntByte = 0; qntByte < bufSize; qntByte++)
                    buf2.push_back(inputSamples[numBuf][data * bufSize + qntByte]);
            buf1.push_back(buf2);
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
        if (queueData.size() > 0)
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

int Queue::getChannelSize()
{
    return FrameSize * bufSize;
}
