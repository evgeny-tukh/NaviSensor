#pragma once

#include <Windows.h>
#include <vector>
#include <functional>

namespace Tools
{
    class Strings: public std::vector <std::string>
    {
        public:
            const int getAsIntAt (const int);
            const char getAsCharAt (const int);
            const double getAsDoubleAt (const int);

            const bool omitted (const int);

            const char *getAt (const int);

        protected:
            const bool isIndexValid (const int);
    };

    typedef std::vector <unsigned int> UIntArray;
    typedef std::vector <in_addr> AddrArray;

    typedef std::function <int (const byte)> SimpleCall;

    Strings *split (const char *source, const char separator);
    Strings& split (Strings& strings, const char *source, const char separator);

    std::string getAppDataFolder (const char *subFolder, const char *subFolder2 = (const char *) 0);

    Strings& getFiles (Strings& files, const char *pattern, const bool storeFullPaths = true);

    Strings& getSerialPortsList (Strings& ports);

    std::string browseForFile (HWND parent, const char *filter, const char *title = "Select file", const char *defaultPath = "");

    AddrArray& getInterfaceList (AddrArray& addrArray);

    void sleepFor (const int millisecods);

    const int decCharToInt (const char);
    const int twoDecCharToInt (const char *digits);
    const int threeDecCharToInt (const char *digits);
    const int fourDecCharToInt (const char *digits);

    const int hexCharToInt (const char);
    const int twoHexCharToInt (const char *digits);

    const double calcDistanceRaftly (const double lat1, const double lon1, const double lat2, const double lon2);

    extern std::string empty;
}

