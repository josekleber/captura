#include "configuration.h"

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

    svFP = pt.get<bool>("FingerPrint.Save");
    BOOST_LOG_TRIVIAL(debug) << "FingerPrint.Save-> " << (svFP ? "On" : "Off");

    mrOn = pt.get<int>("MRServer.On");
    BOOST_LOG_TRIVIAL(debug) << "MRServer-> " << ((mrOn == 1) ? "On" : "Off");

    mrIP = pt.get<std::string>("MRServer.IP");
    BOOST_LOG_TRIVIAL(debug) << "MRServer.IP-> " << mrIP.c_str();

    mrPort = pt.get<std::string>("MRServer.Port");
    BOOST_LOG_TRIVIAL(debug) << "MRServer.Port-> " << mrPort.c_str();

    cutFolder = pt.get<std::string>("AudityInfo.Folder");
    BOOST_LOG_TRIVIAL(debug) << "AudityInfo.Folder-> " << cutFolder.c_str();
}

Configuration::~Configuration()
{
    //dtor
}
