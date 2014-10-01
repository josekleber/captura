#ifndef FILEDATA_H
#define FILEDATA_H

#include <fstream>

#include <boost/filesystem.hpp>

#include "util.h"
#include "parser.h"

using namespace std;

namespace fs = boost::filesystem;

class FileData : public Parser
{
    public:
        /** Default constructor */
        FileData();
        /** Default destructor */
        virtual ~FileData();

        virtual void Execute();

        int32_t idRadio;
    protected:

        int setStreamOut();

        virtual void EndResample();

    private:
//        string cutFolder;

//        int cntFileDayCut;

        /** \brief Grava os dados em disco */
        int Write();
};

#endif // FILEDATA_H
