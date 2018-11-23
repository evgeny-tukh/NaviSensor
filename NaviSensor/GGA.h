#pragma once

#include "Parser.h"

namespace Parsers
{
    class GGA : public NmeaParser
    {
        public:
            GGA ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}