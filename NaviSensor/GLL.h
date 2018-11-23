#pragma once

#include "Parser.h"

namespace Parsers
{
    class GLL : public NmeaParser
    {
        public:
            GLL ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}