#ifndef MAIN_H
#define MAIN_H

/**< controle de versao */
#define prgVersion "1.0.0.0"

#include <iostream>
#include <fstream>

extern "C"
{
    /** inclusão dos headers FFMPEG */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <mir/filter.h>

#include "configuration.h"
#include "database.h"
#include "threadpool.h"
#include "logger.h"

#endif
