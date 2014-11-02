#include "configuration.h"

Configuration::Configuration()
{
    cout << "carregando dados de configuração." << endl;

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("config.ini", pt);

    FilterArqName = pt.get<std::string>("Filters.FileName");

    ConnectionStringSQLProducao = pt.get<std::string>("Database.ConnectionStringSQLProducao");
    ConnectionStringMySQL = pt.get<std::string>("Database.ConnectionStringMYSQL");
    Listener = pt.get<std::string>("Settings.Listener");

    StreamList = pt.get<std::string>("Settings.FileStream");

    UpdateTimer = pt.get<int>("Settings.UpdateTimer");

    svFP = pt.get<bool>("FingerPrint.Save");
    mrOn = pt.get<int>("MRServer.On");

    mrIP = pt.get<std::string>("MRServer.IP");
    mrPort = pt.get<std::string>("MRServer.Port");

    srIP = pt.get<std::string>("SearchResult.IP");
    srPort = pt.get<std::string>("SearchResult.Port");

    cutFolder = pt.get<std::string>("AudityInfo.Folder");

    toFile = pt.get<bool>("Log.toFile");
    toScreen = pt.get<bool>("Log.toScreen");
    onMsg = pt.get<bool>("Log.onMsg");
    onDebug = pt.get<bool>("Log.onDebug");
}

Configuration::~Configuration()
{
    //dtor
}
