#include "configuration.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

Configuration::Configuration()
{
    BOOST_LOG_TRIVIAL(debug) << "carregando dados de configuração.";

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("config.ini", pt);

    FilterArqName = pt.get<std::string>("Filters.FileName");
    BOOST_LOG_TRIVIAL(debug) << "Filters.FileName->" << FilterArqName.c_str();

    ConnectionStringSQL = pt.get<std::string>("Database.ConnectionStringSQL");
    BOOST_LOG_TRIVIAL(debug) << "Database.ConnectionStringSQL-> " << ConnectionStringSQL.c_str();

    ConnectionStringMySQL = pt.get<std::string>("Database.ConnectionStringMYSQL");
    BOOST_LOG_TRIVIAL(debug) << "Database.ConnectionStringMYSQL-> " << ConnectionStringMySQL.c_str();

    Listener = pt.get<std::string>("Settings.Listener");
    BOOST_LOG_TRIVIAL(debug) << "Settings.Listener-> " << Listener.c_str();

    StreamList = pt.get<std::string>("Settings.FileStream");
    BOOST_LOG_TRIVIAL(debug) << "Settings.FileStream-> " << StreamList.c_str();

    UpdateTimer = pt.get<int>("Settings.UpdateTimer");
    BOOST_LOG_TRIVIAL(debug) << "Settings.UpdateTimer-> " << UpdateTimer;

    mrIP = pt.get<std::string>("Socket.IP");
    BOOST_LOG_TRIVIAL(debug) << "Socket.IP-> " << mrIP.c_str();

    mrPort = pt.get<std::string>("Socket.Port");
    BOOST_LOG_TRIVIAL(debug) << "Socket.Port-> " << mrPort.c_str();

    cutFolder = pt.get<std::string>("AudityInfo.Folder");
    BOOST_LOG_TRIVIAL(debug) << "AudityInfo.Folder-> " << cutFolder.c_str();
}

Configuration::~Configuration()
{
    //dtor
}



