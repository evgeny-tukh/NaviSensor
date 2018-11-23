#include "DBT.h"

Parsers::DBT::DBT () : NmeaParser ("DBT")
{
}

bool Parsers::DBT::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    return false;
}