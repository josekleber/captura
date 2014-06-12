#include <iostream>
#include <boost/exception/all.hpp>

using namespace std;


struct exception_base: virtual std::exception, virtual boost:: exception{};

int main()
{
    cout << "Hello world!" << endl;

    return 0;
}
