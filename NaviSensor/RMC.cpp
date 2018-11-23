#include "RMC.h"

Parsers::RMC::RMC () : NmeaParser ("RMC")
{
}

bool Parsers::RMC::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    return false;
}