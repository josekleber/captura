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
/**/
    logging::add_file_log
    (
        keywords::file_name = "a2_%N.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = "[%TimeStamp%]: %Message%"
    );

    logging::add_common_attributes();

//    logging::core::get()->set_filter
//    (
//        logging::trivial::severity == logging::trivial::info
//    );
//    logging::core::get()->set_filter
//    (
//        logging::trivial::severity == logging::trivial::error
//    );
//    logging::core::get()->set_filter
//    (
//        logging::trivial::severity == logging::trivial::trace
//    );
/**/

    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";


    src::logger lg;
    BOOST_LOG(lg) << "Hello, World!";
}
