#pragma once

#include "Parser.h"

namespace Parsers
{
    class DBS : public NmeaParser
    {
        public:
            DBS ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}