#pragma once

#include "Parser.h"

namespace Parsers
{
    class MWD : public NmeaParser
    {
        public:
            MWD ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}