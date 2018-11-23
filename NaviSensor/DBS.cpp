#include "DBS.h"

Parsers::DBS::DBS () : NmeaParser ("DBS")
{
}

bool Parsers::DBS::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    return false;
}