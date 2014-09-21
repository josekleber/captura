#ifndef LOGGER_H
#define LOGGER_H

#include <boost/move/utility.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>



#include <boost/log/support/date_time.hpp>


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;












BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)

class Logger
{
    public:
        /** Default constructor */
        Logger();
        /** Default destructor */
        virtual ~Logger();

        void debug(std::string msg);
        void Teste();
    protected:
    private:
};

#endif // LOGGER_H
