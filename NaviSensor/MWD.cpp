#include "MWD.h"

Parsers::MWD::MWD () : NmeaParser ("MWD")
{
}

bool Parsers::MWD::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields ();
    float           dirTrue, speedKts, speedMps;
    char            trueType, mpsType, ktsType;

    if (Parsers::extractFloat (fields, 1, dirTrue) && Parsers::extractChar (fields, 2, trueType) && trueType == 'T' && fabs (dirTrue) < 360.0f)
        sensor->updateData (Data::DataType::WindDirR, & dirTrue, sizeof (dirTrue));

    if (Parsers::extractFloat (fields, 7, speedMps) && Parsers::extractChar (fields, 8, mpsType) && mpsType == 'M' && speedMps >= 0.0)
    {
        sensor->updateData (Data::DataType::WindSpeedT, & speedMps, sizeof (speedMps));
    }
    else if (Parsers::extractFloat (fields, 5, speedKts) && Parsers::extractChar (fields, 6, ktsType) && ktsType == 'K' && speedKts >= 0.0)
    {
        speedMps = (float) (speedKts * 1852.0 / 3600.0);

        sensor->updateData (Data::DataType::WindSpeedT, &speedMps, sizeof (speedMps));
    }

    return true;
}