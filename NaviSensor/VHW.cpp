#include "VHW.h"

Parsers::VHW::VHW () : NmeaParser ("VHW")
{
}

bool Parsers::VHW::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields ();
    float           heading,
                    speedTW;
    char            headingType,
                    speedType;

    if (Parsers::extractFloat (fields, 1, heading) && Parsers::extractChar (fields, 2, headingType) && headingType == 'T' && fabs (heading) < 360.0f)
        sensor->updateData (Data::DataType::TrueHeading, & heading, sizeof (heading));

    if (Parsers::extractFloat (fields, 5, speedTW) && Parsers::extractChar (fields, 6, speedType) && speedType == 'N' && speedTW > 0.0f && speedTW < 100.0f)
        sensor->updateData (Data::DataType::SpeedTW, & speedTW, sizeof (speedTW));

    return true;
}