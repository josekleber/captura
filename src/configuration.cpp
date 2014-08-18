#include "configuration.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

Configuration::Configuration()
{
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("config.ini", pt);

    FilterArqName = pt.get<std::string>("Filters.FileName");

    ConnectionStringSQL = pt.get<std::string>("Database.ConnectionStringSQL");

    ConnectionStringMySQL = pt.get<std::string>("Database.ConnectionStringMYSQL");

    Listener = pt.get<std::string>("Settings.Listener");
    StreamList = pt.get<std::string>("Settings.FileStream");
    UpdateTimer = pt.get<int>("Settings.UpdateTimer");

    mrIP = pt.get<std::string>("Socket.IP");
    mrPort = pt.get<std::string>("Socket.Port");

    cutFolder = pt.get<std::string>("AudityInfo.Folder");
}

Configuration::~Configuration()
{
    //dtor
}



