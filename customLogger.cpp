
#include "customLogger.hpp"

#include <boost/log/core/core.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>

#include <fstream>
#include <ostream>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

// Declare attribute keywords
BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", logging::trivial::severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(scope, "Scope", attrs::named_scope::value_type)

void coloring_formatter(logging::record_view const& rec, logging::formatting_ostream& strm){

    auto severity  = rec[logging::trivial::severity];
    auto date_time_formatter = expr::stream << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S");
    auto scope_formatter = expr::stream << expr::format_named_scope("Scope" , keywords::format = "%n (%F:%l)", keywords::iteration = expr::reverse );


    date_time_formatter(rec, strm);
    strm << " - ";

    // strm << expr::format_date_time<boost::posix_time::ptime>( "TimeStamp", "%Y-%m-%d, %H:%M:%S.%f") << " - ";

    if (severity)
    {
        // Set the color
        switch (severity.get())
        {
        case logging::trivial::severity_level::info:
            strm << "\033[32m";
            break;
        case logging::trivial::severity_level::warning:
            strm << "\033[33m";
            break;
        case logging::trivial::severity_level::error:
        case logging::trivial::severity_level::fatal:
            strm << "\033[31m";
            break;
        default:
            break;
        }
    }

    // Format the message here...
    strm << "[" << severity << "]";

    if (severity)
    {
        // Restore the default color
        strm << "\033[0m";
    }

    strm << " - ";
    scope_formatter(rec, strm);
    strm << "  ";
    strm << rec[expr::smessage];



}

BOOST_LOG_GLOBAL_LOGGER_INIT(logger, src::severity_logger_mt) {
    src::severity_logger_mt<boost::log::v2_mt_posix::trivial::severity_level> logger;

    // add attributes
    logger.add_attribute("LineID", attrs::counter(1));     // lines are sequentially numbered
    logger.add_attribute("TimeStamp", attrs::local_clock());             // each log line gets a timestamp
    logger.add_attribute("Scope", attrs::named_scope()); 

    // add a text sink
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

    // add a logfile stream to our sink
    sink->locked_backend()->add_stream(
        boost::make_shared< std::ofstream >(LOGFILE));


    // add "console" output stream to our sink
    sink->locked_backend()->add_stream(boost::shared_ptr< std::ostream>(&std::clog, boost::null_deleter()));

    // specify the format of the log message
    /*
    logging::formatter formatter = expr::stream
        << std::setw(7) << std::setfill('0') << line_id << std::setfill(' ') << " | "
        << expr::format_date_time(timestamp, "%Y-%m-%d, %H:%M:%S.%f") << " "
        << "[" << logging::trivial::severity << "]"
        << " - " << expr::smessage;
    */
    sink->set_formatter(&coloring_formatter);

    // just log messages with severity >= SEVERITY_THRESHOLD are written
    sink->set_filter(severity >= SEVERITY_THRESHOLD);


    // "register" our sink
    logging::core::get()->add_sink(sink);

    return logger;

}

