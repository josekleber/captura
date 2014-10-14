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

void Logger::Teste()
{
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";


    src::logger lg;
    BOOST_LOG(lg) << "Hello, World!";
}
