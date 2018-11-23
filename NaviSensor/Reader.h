#pragma once

#include "../NaviSensorUI/SensorConfig.h"
#include "BinQueue.h"

namespace Readers
{
    class Reader
    {
        public:
            Reader (Sensors::Config *config);
            ~Reader ();

            virtual size_t read ();
            virtual bool open ();
            virtual void close ();

            size_t getData (byte *buffer, const size_t size);
            size_t getData (char *buffer, const size_t size, const char *eol);

        protected:
            bool             opened;
            Sensors::Config *config;
            BinaryQueue      queue;
    };

}