#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <iostream>

using namespace std;

class Configuration
{
    public:
        Configuration();

        std::string FilterArqName;
        std::string ConnectionString;
        std::string Listener;

        std::string mrIP;
        std::string mrPort;

        std::string cutFolder;

        virtual ~Configuration();

    protected:

    private:

};

#endif // CONFIGURATION_H
