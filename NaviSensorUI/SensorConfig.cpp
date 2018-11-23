#include "SensorConfig.h"
#include "tools.h"

#define SECTION_SENSOR  "Sensor"
#define SECTION_SERIAL  "Serial"
#define SECTION_UDP     "UDP"
#define SECTION_FILE    "File"

std::string Sensors::Config::cfgFolder = Tools::getAppDataFolder ("CAIM", "NaviSensor");

Sensors::NamedOptions Sensors::stopBitOptions    { { Sensors::One, "1" }, { Sensors::OneAndHalf, "1.5" }, { Sensors::Two, "2" } };
Sensors::NamedOptions Sensors::parityOptions     { { Sensors::None, "None" }, { Sensors::Odd, "Odd" }, { Sensors::Even, "Even" }, { Sensors::Mark, "Mark" },
                                                   { Sensors::Space, "Space" } };
Sensors::NamedOptions Sensors::sensorTypeOptions { { Sensors::Type::NMEA, "NMEA" }, { Sensors::Type::AIS, "AIS" } };
Sensors::NamedOptions Sensors::connTypeOptions   { { Sensors::Connection::File, "File" }, { Sensors::Connection::Serial, "Serial" },
                                                   { Sensors::Connection::UDP, "UDP" } };

void Sensors::Config::setCfgFileName (const char *cfgFileName)
{
    this->cfgFileName = cfgFileName;
}

void Sensors::Config::assign (Sensors::Config& source)
{
    cfgFileName = source.cfgFileName.c_str ();
}

Sensors::Config& Sensors::Config::operator = (Sensors::Config& source)
{
    assign (source);

    return *this;
}

const char *Sensors::Config::getCfgFilePath ()
{
    cfgFilePath = cfgFolder + std::string("\\") + cfgFileName;

    return cfgFilePath.c_str ();
}

void Sensors::Config::deleteCfgFile ()
{
    remove (getCfgFilePath ());
}

void Sensors::Config::saveData (const char *section, const char *key, std::string& value)
{
    WritePrivateProfileString(section, key, value.c_str (), getCfgFilePath ());
}

void Sensors::Config::saveData (const char *section, const char *key, const char *value)
{
    WritePrivateProfileString (section, key, value, getCfgFilePath ());
}

void Sensors::Config::saveData (const char *section, const char *key, const int value)
{
    char buffer [100];

    saveData (section, key, _itoa (value, buffer, 10));
}

void Sensors::Config::saveData (const char *section, const char *key, const double value)
{
    char buffer[100];

    sprintf (buffer, "%f", value);

    saveData (section, key, buffer);
}

std::string Sensors::Config::loadData (const char *section, const char *key, const char *defValue)
{
    char buffer [1000];

    GetPrivateProfileString (section, key, defValue, buffer, sizeof (buffer), getCfgFilePath ());

    return std::string (buffer);
}

int Sensors::Config::loadNumericData (const char *section, const char *key, const int defValue)
{
    char        defBuffer [50];
    std::string data = loadData (section, key, _itoa(defValue, defBuffer, 10));

    return atoi (data.c_str ());
}

double Sensors::Config::loadFloatData (const char *section, const char *key, const double defValue)
{
    char        defBuffer[50];
    std::string data;
    
    sprintf (defBuffer, "%f", defValue);

    data = loadData(section, key, defBuffer);

    return atof (data.c_str ());
}

Sensors::SerialParams::SerialParams () : Sensors::Params ()
{
    baud     = 4800;
    port     = 1;
    byteSize = 8;
    parity   = None;
    stopBits = One;
}

void Sensors::SerialParams::assign (Sensors::SerialParams& source)
{
    baud        = source.baud;
    byteSize    = source.byteSize;
    parity      = source.parity;
    port        = source.port;
    stopBits    = source.stopBits;

    Config::assign (source);
}

void Sensors::SerialParams::load ()
{
    port     = loadNumericData (SECTION_SERIAL, "Port", 1);
    baud     = loadNumericData (SECTION_SERIAL, "Baud", 4800);
    byteSize = loadNumericData (SECTION_SERIAL, "ByteSize", 8);
    parity   = (Parity) loadNumericData (SECTION_SERIAL, "Parity", None);
    stopBits = (StopBits) loadNumericData (SECTION_SERIAL, "StopBits", One);
}

void Sensors::SerialParams::save ()
{
    saveData (SECTION_SERIAL, "Port", port);
    saveData (SECTION_SERIAL, "Baud", baud);
    saveData (SECTION_SERIAL, "ByteSize", byteSize);
    saveData (SECTION_SERIAL, "Parity", parity);
    saveData (SECTION_SERIAL, "StopBits", stopBits);
}

std::string Sensors::SerialParams::getParameterString ()
{
    char buffer [100];

    sprintf (buffer, "COM%d, %d bit, %s, %s stop bit", port, byteSize, getOptionName (parityOptions, parity), getOptionName (stopBitOptions, stopBits));

    return std::string (buffer);
}

Sensors::UdpParams::UdpParams () : Sensors::Params ()
{
    inPort  = 8010;
    outPort = 8011;

    bind.S_un.S_addr = INADDR_ANY;
    dest.S_un.S_addr = INADDR_BROADCAST;
}

void Sensors::UdpParams::assign (Sensors::UdpParams& source)
{
    inPort           = source.inPort;
    outPort          = source.outPort;
    bind.S_un.S_addr = source.bind.S_un.S_addr;
    dest.S_un.S_addr = source.dest.S_un.S_addr;

    Sensors::Config::assign (source);
}

void Sensors::UdpParams::load ()
{
    std::string bindAddr = loadData (SECTION_UDP, "BindAddr", "");
    std::string destAddr = loadData (SECTION_UDP, "DestAddr", "255.255.255.255");

    inPort  = loadNumericData (SECTION_UDP, "InPort", 8010);
    outPort = loadNumericData (SECTION_UDP, "OutPort", 8011);

    bind.S_un.S_addr = inet_addr (bindAddr.c_str ());
    dest.S_un.S_addr = inet_addr (destAddr.c_str ());
}

void Sensors::UdpParams::save ()
{
    saveData (SECTION_UDP, "InPort", inPort);
    saveData (SECTION_UDP, "OutPort", outPort);
    saveData (SECTION_UDP, "BindAddr", inet_ntoa (bind));
    saveData (SECTION_UDP, "DestAddr", inet_ntoa (dest));
}

std::string Sensors::UdpParams::getParameterString ()
{
    char buffer [200];

    sprintf (buffer, "[%s] %d <=> [%s] %d", inet_ntoa (bind), inPort, inet_ntoa (dest), outPort);

    return std::string (buffer);
}

Sensors::FileParams::FileParams () : Params ()
{
    filePath         = Tools::empty;
    pauseBetwenLines = 100;
}

void Sensors::FileParams::assign(Sensors::FileParams& source)
{
    filePath         = source.filePath.c_str ();
    pauseBetwenLines = source.pauseBetwenLines;

    Config::assign (source);
}

void Sensors::FileParams::load ()
{
    filePath         = loadData (SECTION_FILE, "Path", "");
    pauseBetwenLines = loadNumericData (SECTION_FILE, "PauseBetweenLines", 50);
}

void Sensors::FileParams::save ()
{
    saveData (SECTION_FILE, "Path", filePath);
    saveData (SECTION_FILE, "PauseBetweenLines", pauseBetwenLines);
}

std::string Sensors::FileParams::getParameterString ()
{
    char buffer[1000];

    sprintf (buffer, "%s, %dms", filePath.c_str (), pauseBetwenLines);

    return std::string (buffer);
}

Sensors::SensorConfig::SensorConfig (const int id) : Config ()
{
    name         = "New sensor";
    type         = NMEA;
    connection   = Serial;
    pauseBtwIter = 10;

    setID (id);
}

void Sensors::SensorConfig::setID (const int sensorID)
{
    this->sensorID = sensorID;

    if (sensorID > 0)
    {
        char fileName [100];

        sprintf (fileName, "config_%03d.cfg", sensorID);

        setCfgFileName (fileName);
    }
}

void Sensors::SensorConfig::setCfgFileName (const char *cfgFileName)
{
    Config::setCfgFileName (cfgFileName);

    serialParam.setCfgFileName (cfgFileName);
    udpParam.setCfgFileName (cfgFileName);
    fileParam.setCfgFileName (cfgFileName);
}

void Sensors::SensorConfig::assign (Sensors::SensorConfig& source)
{
    connection   = source.connection;
    name         = source.name.c_str ();
    sensorID     = source.sensorID;
    type         = source.type;
    serialParam  = source.serialParam;
    udpParam     = source.udpParam;
    pauseBtwIter = source.pauseBtwIter;
    fileParam    = source.fileParam;

    Sensors::Config::assign (source);
}

void Sensors::SensorConfig::save ()
{
    saveData (SECTION_SENSOR, "Name", name);
    saveData (SECTION_SENSOR, "Type", type);
    saveData (SECTION_SENSOR, "Connection", connection);
    saveData (SECTION_SENSOR, "PauseBtwIter", pauseBtwIter);
    
    serialParam.save ();
    udpParam.save ();
    fileParam.save ();
}

void Sensors::SensorConfig::load ()
{
    name         = loadData (SECTION_SENSOR, "Name", "New sensor");
    type         = (Type) loadNumericData (SECTION_SENSOR, "Type", NMEA);
    connection   = (Connection) loadNumericData (SECTION_SENSOR, "Connection", Serial);
    pauseBtwIter = loadNumericData (SECTION_SENSOR, "PauseBtwIter", 10);

    serialParam.load ();
    udpParam.load ();
    fileParam.load ();
}

std::string Sensors::SensorConfig::getParameterString ()
{
    std::string result;

    switch (connection)
    {
        case Sensors::Connection::File:
            result = fileParam.getParameterString (); break;

        case Sensors::Connection::Serial:
            result = serialParam.getParameterString (); break;

        case Sensors::Connection::UDP:
            result = udpParam.getParameterString(); break;

        default:
            result = Tools::empty;
    }

    return result;
}

Sensors::SensorConfigArray::SensorConfigArray (const bool load)
{
    if (load)
        loadAll ();
}

Sensors::SensorConfigArray::~SensorConfigArray ()
{
    for (auto config : *this)
    {
        if (config)
            delete config;
    }
}

void Sensors::SensorConfigArray::enumElements (std::function <bool (Sensors::SensorConfig *, void *)> callback, void *param)
{
    for (auto config : *this)
    {
        if (config && !callback (config, param))
            break;
    }
}

const bool Sensors::SensorConfigArray::isIdUsed (const int id)
{
    bool used = false;

    for (auto config : *this)
    {
        if (config && config->id () == id)
        {
            used = true; break;
        }
    }

    return used;
}

const int Sensors::SensorConfigArray::findUnusedId ()
{
    int id;

    for (id = 1; isIdUsed (id); ++ id);

    return id;
}

Sensors::SensorConfig *Sensors::SensorConfigArray::createEmptyConfig ()
{
    return new Sensors::SensorConfig (findUnusedId ());
}

void Sensors::SensorConfigArray::DeleteConfigByIndex (const size_t index, const bool removeCfgFile)
{
    if (index >= 0 && index < size ())
    {
        iterator              pos    = begin ();
        Sensors::SensorConfig *config = at (index);

        if (config)
        {
            if (removeCfgFile)
                config->deleteCfgFile ();
                
            delete config;
        }

        advance (pos, index);

        erase (pos);
    }
}

void Sensors::SensorConfigArray::DeleteConfigByID (const int id, const bool removeCfgFile)
{
    for (iterator pos = begin (); pos != end (); ++ pos)
    {
        SensorConfig *config = *pos;

        if (config && config->id() == id)
        {
            if (removeCfgFile)
                config->deleteCfgFile();

            delete config;

            erase (pos);
            
            break;
        }
    }
}

Sensors::SensorConfig *Sensors::SensorConfigArray::addNew (const int id)
{
    SensorConfig *config = id == 0 ? createEmptyConfig () : new SensorConfig (id);

    push_back (config);

    return config;
}

Sensors::SensorConfig *Sensors::SensorConfigArray::addFrom (Sensors::SensorConfig& source)
{
    int           id     = findUnusedId ();
    SensorConfig *config = new SensorConfig ();

    config->assign (source);
    config->setID (id);

    push_back (config);

    return config;
}

void Sensors::SensorConfigArray::loadAll ()
{
    std::string    pattern = Tools::getAppDataFolder ("CAIM", "NaviSensor") + "\\config_???.cfg";
    Tools::Strings files;

    Tools::getFiles (files, pattern.c_str ());

    for (auto & file : files)
    {
        int id = atoi (file.substr (file.length () - 7, 3).c_str ());

        SensorConfig *config = addNew (id);

        config->load ();
    }
}

void Sensors::SensorConfigArray::saveAll ()
{
    for (auto config : *this)
    {
        if (config)
            config->save ();
    }
}

const char *Sensors::getOptionName (Sensors::NamedOptions& options, const unsigned int value)
{
    const char *result = (const char *) 0;

    for (auto & option : options)
    {
        if (option.first == value)
        {
            result = option.second; break;
        }
    }

    return result;
}
