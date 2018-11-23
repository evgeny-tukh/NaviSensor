#pragma once

#include "Parser.h"

namespace Parsers
{
    class MWV : public NmeaParser
    {
        public:
            MWV ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}