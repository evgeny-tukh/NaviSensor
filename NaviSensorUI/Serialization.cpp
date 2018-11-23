#include "Serialization.h"
#include "Tools.h"

std::string Serialization::empty = "";

void Serialization::Serializable::serialize(Serialization::Worker& worker)
{
    for (std::string& key : keys)
        worker.write(key, getWorkerParam(), getValue(key));
}

void Serialization::Serializable::deserialize(Serialization::Worker& worker)
{
    for (std::string& key : keys)
        setValue (key, (worker.read(key, getWorkerParam ())));
}

Serialization::IniFile::IniFile (const std::string& filePath) : Worker ()
{
    setFilePath (filePath);
}

void Serialization::IniFile::setFilePath (const std::string& filePath)
{
    this->filePath = filePath;
}

const std::string& Serialization::IniFile::read (const std::string& key, const void *param)
{
    const char     *keyStr   = key.c_str();
    Tools::Strings *parts    = Tools::split(keyStr, ':');
    const char     *defValue = (parts->size () > 2) ? parts->at (2).c_str () : "";
    char            buffer [1000];

    ::GetPrivateProfileString (parts->at(0).c_str(), parts->at(1).c_str(), defValue, buffer, sizeof (buffer), (const char *) param);

    return *(new std::string (buffer));
}

void Serialization::IniFile::write (const std::string& key, const void *param, const std::string& value)
{
    const char     *keyStr = key.c_str();
    Tools::Strings *parts  = Tools::split (keyStr, ':');

    ::WritePrivateProfileString (parts->at (0).c_str (), parts->at (1).c_str(), value.c_str (), (const char *) param);
}
