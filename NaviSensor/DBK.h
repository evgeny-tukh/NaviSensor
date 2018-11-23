#pragma once

#include "Parser.h"

namespace Parsers
{
    class DBK : public NmeaParser
    {
        public:
            DBK ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}