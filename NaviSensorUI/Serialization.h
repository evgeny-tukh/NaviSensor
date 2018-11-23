#pragma once

#include <string>
#include <vector>
#include <winsock.h>
#include <functional>

namespace Serialization
{
    extern std::string empty;

    class Worker
    {
        public:
            Worker () {}

            virtual const std::string& read (const std::string& key, const void *param) { return empty; }
            virtual void write (const std::string& key, const void *param, const std::string& value) {}
    };

    class IniFile : public Worker
    {
        public:
            IniFile (const std::string& filePath = "");

            void setFilePath (const std::string& filePath);

            virtual const std::string& read (const std::string& key, const void *param);
            virtual void write (const std::string& key, const void *param, const std::string& value);

        protected:
            std::string filePath;
    };

    class Serializable
    {
        public:
            Serializable () {}

            void serialize (Worker& worker);
            void deserialize (Worker& worker);

        protected:
            std::vector <std::string> keys;

            virtual const std::string& loader (const std::string& key, const void *object) { return empty; }
            virtual void saver (std::string& key, const std::string& value, const void *object) {}

            virtual const void *getWorkerParam ()
            {
                return (const void *) 0;
            }

            virtual const std::string& getValue (const std::string& key)
            {
                return empty;
            }

            virtual void setValue (const std::string& key, const std::string& value)
            {
            }
    };
}

