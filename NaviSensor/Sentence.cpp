#include <string>
#include "Sentence.h"

NMEA::Sentence::Sentence (const char *source)
{
    parse (source);
}

void NMEA::Sentence::parse (const char *source)
{
    parsed = true;

    switch (source [0])
    {
        case '$':
            sixBitEncoded = false; break;

        case '!':
            sixBitEncoded = true; break;

        default:
            parsed = false;
    }

    if (parsed && !checkCRC (source))
        parsed = false;

    if (parsed)
    {
        char payload [100], *from, *to;

        memset (payload, 0, sizeof (payload));

        for (from = (char *) source + 1, to = payload; *from && *from != '*'; *(to ++) = *(from ++));

        Tools::split (fields, payload, ',');

        if (fields.size() > 1)
        {
            std::string& signature = fields [0];
            const char  *signStr   = signature.c_str ();
            char         typeBuf [3];

            proprietary = signStr [0] == 'P' && strlen (signStr) == 6;

            if (proprietary)
            {
                memcpy (typeBuf, signStr + 3, 3);
                memcpy (talkerID, signStr + 1, 2);
            }
            else
            {
                memcpy (typeBuf, signStr + 2, 3);
                memcpy (talkerID, signStr, 2);
            }

            type = typeBuf;
        }
        else
        {
            parsed = false;
        }
    }
}

bool NMEA::Sentence::checkCRC (const char *source)
{
    byte  crc;
    char *curChar;
    bool  result;

    for (crc = (byte) source [1], curChar = (char *) source + 2; *curChar && *curChar != '*'; crc ^= (byte) *(curChar ++));

    if (*curChar == '*')
        result = Tools::twoHexCharToInt (curChar + 1) == crc;
    else
        result = false;

    return result;
}

Tools::Strings& NMEA::Sentence::getFields ()
{
    return fields;
}

const char *NMEA::Sentence::getTypeName ()
{
    return type.name;
}

const int NMEA::Sentence::getType ()
{
    return type.type;
}
