#pragma once

#include "Parser.h"

namespace Parsers
{
    class DBc : public NmeaParser
    {
        public:
            DBc (const char *);

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);

        protected:
            Data::DataType dataType;
    };
};
