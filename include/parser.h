#ifndef PARSER_H
#define PARSER_H

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

using namespace std;

class Parser
{
    public:
        /** Default constructor */
        Parser();
        /** Default destructor */
        virtual ~Parser();

        /** \brief
        * Converte o audio
        */
        void Execute();
    protected:
    private:
};

#endif // PARSER_H
