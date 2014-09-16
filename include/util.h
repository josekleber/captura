#ifndef UTIL_H
#define UTIL_H

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

class util
{
    public:
        util() {}
        virtual ~util() {}
    protected:
    private:
};

#endif // UTIL_H
