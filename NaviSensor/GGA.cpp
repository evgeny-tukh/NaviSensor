#include "Parameters.h"
#include "GGA.h"

Parsers::GGA::GGA () : NmeaParser ("GGA")
{
}

bool Parsers::GGA::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields ();
    Data::Time      utc;
    Data::Pos       position;
    float           hdop;
    byte            gpsQuality;

    if (Parsers::extractUTC (fields, 1, utc))
        sensor->updateData (Data::DataType::UTC, & utc, sizeof (utc));

    if (Parsers::extractPosition (fields, 2, position))
        sensor->updateData (Data::DataType::Position, & position, sizeof (position));

    if (Parsers::extractByte (fields, 6, gpsQuality))
        sensor->updateData (Data::DataType::GPSQual, & gpsQuality, sizeof (gpsQuality));

    if (Parsers::extractFloat (fields, 8, hdop))
        sensor->updateData (Data::DataType::HDOP, & hdop, sizeof (hdop));

    return true;
}