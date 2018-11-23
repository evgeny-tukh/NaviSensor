#include "VTG.h"

Parsers::VTG::VTG () : NmeaParser ("VTG")
{
}

bool Parsers::VTG::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields();
    float           course,
                    speedOG;
    char            courseType,
                    speedType,
                    modeIndicator;

    if (Parsers::extractFloat (fields, 1, course) && Parsers::extractChar (fields, 2, courseType) && courseType == 'T' && fabs (course) < 360.0f)
        sensor->updateData (Data::DataType::Course, & course, sizeof (course));

    if (Parsers::extractFloat (fields, 5, speedOG) && Parsers::extractChar (fields, 6, speedType) && speedType == 'N' && speedOG > 0.0f && speedOG < 100.0f)
        sensor->updateData (Data::DataType::SpeedOG, & speedOG, sizeof (speedOG));

    if (Parsers::extractChar (fields, 9, modeIndicator))
        sensor->updateData (Data::DataType::PosSysMode, & modeIndicator, sizeof (modeIndicator));

    return true;
}