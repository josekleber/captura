#ifndef FILEDATA_H
#define FILEDATA_H

#include "parser.h"
#include <fstream>
#include <boost/filesystem.hpp>

using namespace std;

namespace fs = boost::filesystem;

class FileData : public Parser
{
    public:
        /** Default constructor */
        FileData();
        FileData(string fileName, uint64_t channelLayoutIn, int sampleRateIn,
                 int bitRateIn, AVSampleFormat sampleFormatIn, int nbSamplesIn, int nbChannelIn);
        /** Default destructor */
        virtual ~FileData();

        virtual void Execute();

        string fileName;
    protected:

        int setStreamOut();

        virtual void EndResample();

    private:

        string cutFolder;

        int cntFileDayCut;

        /** \brief Grava os dados em disco */
        int Write();
};

#endif // FILEDATA_H
