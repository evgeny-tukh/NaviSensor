#pragma once

#include "../NaviSensorUI/tools.h"
#include "Sensor.h"
#include <functional>

namespace NMEA
{
    union SentenceType
    {
        char         name [4];
        unsigned int type;

        SentenceType ()
        {
            type = 0;
        }

        SentenceType (const char *typeString)
        {
            *this = typeString;
        }

        SentenceType& operator = (const char *sourceType)
        {
            type = 0;

            memcpy (name, sourceType, 3);

            return *this;
        }
    };

    class Sentence
    {
        public:
            Sentence (const char *source);

            Tools::Strings& getFields ();

            const char *getTypeName ();
            const int getType ();

        protected:
            SentenceType   type;
            char           talkerID [2];
            bool           proprietary;
            bool           parsed;
            bool           sixBitEncoded;
            Tools::Strings fields;

            void parse (const char *source);

            bool checkCRC (const char *source);
    };
}
