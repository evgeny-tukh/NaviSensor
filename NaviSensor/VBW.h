#pragma once

#include "Parser.h"

namespace Parsers
{
    class VBW : public NmeaParser
    {
        public:
            VBW ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}