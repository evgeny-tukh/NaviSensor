#pragma once

#include <string>
#include <vector>
#include <winsock.h>
#include <functional>
#include "Serialization.h"
#include "tools.h"

#ifndef MAX_PATH
    #define MAX_PATH    1000
#endif

namespace Sensors
{
    constexpr auto MAX_NAME = 20;

    typedef std::vector <std::pair <unsigned int, const char *>> NamedOptions;

    typedef enum
    {
        NMEA = 1,
        AIS  = 2
    }
    Type;

    typedef enum
    {
        Serial = 1,
        UDP    = 2,
        File   = 3
    }
    Connection;

    enum Parity
    {
        None  = PARITY_NONE,
        Odd   = PARITY_ODD,
        Even  = PARITY_EVEN,
        Mark  = PARITY_MARK,
        Space = PARITY_SPACE
    };

    enum StopBits
    {
        One        = ONESTOPBIT,
        OneAndHalf = ONE5STOPBITS,
        Two        = TWOSTOPBITS
    };

    class Config
    {
        public:
            Config () {}

            Config& operator = (Config& source);

            virtual void assign (Config& source);

            virtual void save () {}
            virtual void load () {}

            void setCfgFileName (const char *cfgFileName);

            void deleteCfgFile();

        protected:
            std::string cfgFileName;
            std::string cfgFilePath;

            static std::string cfgFolder;

            void saveData (const char *section, const char *key, std::string& value);
            void saveData (const char *section, const char *key, const char *value);
            void saveData (const char *section, const char *key, const int value);
            void saveData (const char *section, const char *key, const double value);
            std::string loadData (const char *section, const char *key, const char *defValue);
            int loadNumericData (const char *section, const char *key, const int defValue);
            double loadFloatData (const char *section, const char *key, const double defValue);

            const char *getCfgFilePath ();
    };

    class Params : public Config
    {
        public:
            virtual std::string getParameterString () { return Tools::empty; }
    };

    class SerialParams : public Params
    {
        public:
            SerialParams ();

            SerialParams& operator = (SerialParams& source)
            {
                assign (source); return *this;
            }

            virtual void assign (SerialParams& source);

            virtual void save ();
            virtual void load ();

            virtual std::string getParameterString ();

            int    port;
            int    baud;
            int    byteSize;
            Parity parity;
            int    stopBits;
    };

    class UdpParams : public Params
    {
        public:
            UdpParams ();

            UdpParams& operator = (UdpParams& source)
            {
                assign (source); return *this;
            }

            virtual void assign (UdpParams& source);

            virtual void save();
            virtual void load();

            virtual std::string getParameterString ();

            int     inPort;
            int     outPort;
            in_addr bind;
            in_addr dest;
    };

    class FileParams : public Params
    {
        public:
            FileParams ();

            FileParams& operator = (FileParams& source)
            {
                assign (source); return *this;
            }

            virtual void assign (FileParams& source);

            virtual void save ();
            virtual void load ();

            virtual std::string getParameterString ();

            std::string filePath;
            int         pauseBetwenLines;
    };

    class SensorConfig : public Config
    {
        public:
            SensorConfig (const int id = 0);

            SensorConfig& operator = (SensorConfig& source)
            {
                assign (source); return *this;
            }

            virtual void assign (SensorConfig& source);

            void setCfgFileName (const char *cfgFileName);

            virtual void save ();
            virtual void load ();

            void setID (const int sensorID);

            inline const int id ()
            {
                return sensorID;
            }

            inline const char *getName ()
            {
                return name.c_str ();
            }

            std::string getParameterString ();

            int          sensorID;
            int          pauseBtwIter;
            std::string  name;
            Type         type;
            Connection   connection;
            SerialParams serialParam;
            UdpParams    udpParam;
            FileParams   fileParam;
    };

    class SensorConfigArray : public std::vector <SensorConfig *>
    {
        public:
            SensorConfigArray (const bool load = false);
            virtual ~SensorConfigArray ();

            SensorConfig *addNew (const int id = 0);
            SensorConfig *addFrom (SensorConfig& source);

            SensorConfig *createEmptyConfig ();

            void DeleteConfigByIndex (const size_t index, const bool removeCfgFile = false);
            void DeleteConfigByID (const int id, const bool removeCfgFile = false);

            void enumElements (std::function <bool (SensorConfig *, void *)>, void *param);

            void loadAll ();
            void saveAll ();

        protected:
            const bool isIdUsed (const int id);
            const int findUnusedId ();
    };

    extern NamedOptions stopBitOptions, parityOptions, sensorTypeOptions, connTypeOptions;

    const char *getOptionName (NamedOptions& options, const unsigned int value);
}
