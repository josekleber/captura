#include "logger.h"

Logger::Logger()
{
    //ctor
}

Logger::~Logger()
{
    //dtor
}

void Logger::debug(std::string msg)
{
    src::logger_mt& lg = my_logger::get();
    BOOST_LOG(lg) << msg;
}
