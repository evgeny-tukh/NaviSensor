#include <sstream>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "tools.h"
#include <Windows.h>
#include <Shlwapi.h>
#include <Shlobj.h>

namespace Tools
{
    std::string empty = "";

    Strings& split (Strings& parts, const char *source, const char separator)
    {
        std::istringstream stream (source);
        std::string        line;

        while (getline (stream, line, separator))
        {
            //std::cout << line << std::endl;

            parts.push_back (line);
        }

        return parts;
    }

    Strings *split (const char *source, const char separator)
    {
        Strings           *parts = new Strings;
        std::istringstream stream (source);
        std::string        line;

        while (getline (stream, line, separator))
        {
            std::cout << line << std::endl;

            parts->push_back (line);
        }

        return parts;
    }

    std::string getAppDataFolder (const char *subFolder, const char *subFolder2)
    {
        char path [MAX_PATH];

        if (SHGetFolderPath(HWND_DESKTOP, CSIDL_COMMON_APPDATA, NULL, 0, path) != S_OK)
        {
            GetModuleFileName (NULL, path, sizeof (path));
            PathRemoveFileSpec (path);
        }

        PathAppend (path, subFolder);
        CreateDirectory (path, NULL);

        if (subFolder2)
        {
            PathAppend (path, subFolder2);
            CreateDirectory (path, NULL);
        }

        return path;
    }

    Strings& getFiles (Strings& files, const char *pattern, const bool storeFullPaths)
    {
        WIN32_FIND_DATA findData;
        HANDLE          findHandle = FindFirstFile (pattern, & findData);

        files.clear ();

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                char path [MAX_PATH];

                if (storeFullPaths)
                {
                    strcpy (path, pattern);
                    PathRemoveFileSpec (path);
                    PathAppend  (path, findData.cFileName);
                }
                else
                {
                    strcpy (path, findData.cFileName);
                }

                files.push_back (path);
            }
            while (FindNextFile (findHandle, & findData));

            FindClose (findHandle);
        }

        return files;
    }

    Strings& getSerialPortsList (Strings& ports)
    {
        HKEY  scomKey;
        int   count = 0;
        DWORD error = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Hardware\\DeviceMap\\SerialComm", 0, KEY_QUERY_VALUE, & scomKey);

        if (error == S_OK)
        {
            char  valueName[100],
                  valueValue[100];
            DWORD nameSize,
                  valueSize,
                  valueType;
            BOOL  valueFound;

            do
            {
                nameSize   = sizeof (valueName);
                valueSize  = sizeof (valueValue);
                valueFound = RegEnumValue (scomKey, (DWORD) count ++, valueName, & nameSize, NULL, & valueType, (BYTE *) valueValue, & valueSize) == S_OK;

                if (valueFound)
                    ports.push_back (valueValue);
            } while (valueFound);

            RegCloseKey (scomKey);
        }

        return ports;
    }

    std::string browseForFile (HWND parent, const char *filter, const char *title, const char *defaultPath)
    {
        OPENFILENAME data;
        char         buffer [MAX_PATH],
                     filterBuf [500];
        std::string  result;

        if (defaultPath)
            strcpy (buffer, defaultPath);

        if (filter)
            strcpy (filterBuf, filter);
        else
            memset (filterBuf, 0, sizeof (filterBuf));

        memset (& data, 0, sizeof (data));

        data.lStructSize = sizeof (data);
        data.hwndOwner   = parent;
        data.hInstance   = (HINSTANCE) GetWindowLong (parent, GWL_USERDATA);
        data.lpstrFilter = filterBuf;
        data.lpstrFile   = buffer;
        data.nMaxFile    = sizeof (buffer);
        data.lpstrTitle  = title;
        data.Flags       = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

        for (char *ptr = (char *) filterBuf; *ptr; ++ ptr)
        {
            if (*ptr == '|')
                *ptr = '\0';
        }

        if (GetOpenFileName (& data))
            result = buffer;
        else
            result = Tools::empty;

        return result;
    }

    AddrArray& getInterfaceList (AddrArray& addrArray)
    {
        char     host [100];
        hostent *hostEnt;
        in_addr  addr;

        gethostname (host, sizeof (host));

        hostEnt = gethostbyname (host);

        for (int i = 0; hostEnt->h_addr_list [i] && hostEnt->h_addr_list [i][0]; ++ i)
        {
            addr.S_un.S_addr = ((in_addr *) hostEnt->h_addr_list [i])->S_un.S_addr;

            addrArray.push_back (addr);
        }

        return addrArray;
    }

    void sleepFor (const int millisecods)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (millisecods));
    }

    const int decCharToInt (const char digit)
    {
        return (digit >= '0' && digit <= '9') ? (digit - '0') : 0;
    }

    const int twoDecCharToInt (const char *digits)
    {
        return (decCharToInt (digits [0]) * 10) + decCharToInt (digits [1]);
    }

    const int threeDecCharToInt (const char *digits)
    {
        return (decCharToInt (digits [0]) * 100) + (decCharToInt (digits [1]) * 10) + decCharToInt (digits [2]);
    }

    const int fourDecCharToInt(const char *digits)
    {
        return (decCharToInt (digits [0]) * 1000) + (decCharToInt (digits[1]) * 100) + (decCharToInt(digits [2]) * 10) + decCharToInt(digits [3]);
    }

    const int hexCharToInt (const char digit)
    {
        int result;

        if (digit >= '0' && digit <= '9')
            result = digit - '0';
        else if (digit >= 'a' && digit <= 'f')
            result = digit - 'a' + 10;
        else if (digit >= 'A' && digit <= 'F')
            result = digit - 'A' + 10;
        else
            result = 0;

        return result;
    }

    const int twoHexCharToInt (const char *digits)
    {
        return (hexCharToInt (digits [0]) << 4) + hexCharToInt (digits [1]);
    }
}

const int Tools::Strings::getAsIntAt (const int index)
{
    return atoi (getAt (index));
}

const char Tools::Strings::getAsCharAt (const int index)
{
    return *getAt (index);
}

const double Tools::Strings::getAsDoubleAt (const int index)
{
    return atof (getAt (index));
}

const char *Tools::Strings::getAt (const int index)
{
    return isIndexValid (index) ? at (index).c_str () : "";
}

const bool Tools::Strings::omitted (const int index)
{
    return !*getAt (index);
}

const bool Tools::Strings::isIndexValid (const int index)
{
    return index >= 0 && (unsigned) index < size ();
}
