#include "DBc.h"

Parsers::DBc::DBc (const char *sentenceType) : NmeaParser (sentenceType)
{
    if (sentenceType [0] == 'D' && sentenceType [1] == 'B')
    {
        switch (sentenceType[2])
        {
            case 'K':
                dataType = Data::DataType::DepthBK; break;

            case 'S':
                dataType = Data::DataType::DepthBS; break;

            case 'T':
                dataType = Data::DataType::DepthBT; break;

            default:
                dataType = Data::DataType::Unknown;
        }
    }
}

bool Parsers::DBc::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    bool            parsed = false;
    Tools::Strings& fields = sentence.getFields();
    auto            calc = [fields] (const unsigned int start, const char symbol, const double multiplier) -> double
                           {
                               double result = -1;

                               if (fields.size () > (start + 2))
                               {
                                   const char *depthChar = fields [start+1].c_str (),
                                              *depthVal  = fields [start].c_str ();

                                   if (*depthChar == symbol && *depthVal)
                                       result = atof (depthVal) * multiplier;
                                   else
                                       result = -1;
                               }

                               return result;
                           };
    double          depthFeet    = calc (1, 'f', 0.3048),
                    depthMeters  = calc (3, 'M', 1.0),
                    depthFathoms = calc (5, 'F', 1.8288),
                    result;

    if (depthMeters > 0.0)
        result = depthMeters;
    else if (depthFeet > 0.0)
        result = depthFeet;
    else if (depthFathoms > 0.0)
        result = depthFathoms;
    else
        result = -1;

    if (result > 0.0)
    {
        float resFloat = (float) result;

        sensor->updateData (dataType, & resFloat, sizeof (resFloat));

        parsed = true;
    }

    return parsed;
}