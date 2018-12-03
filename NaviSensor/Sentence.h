#pragma once

#include "../NaviSensorUI/tools.h"
#include <functional>
#include <map>

namespace NMEA
{
    union SentenceType
    {
        #pragma pack(1)
        char         name [4];
        unsigned int type;
        #pragma pack()

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

    #pragma pack(1)

    struct SentenceStatus
    {
        SentenceType type;
        time_t       lastReceived;
        unsigned int count;

        SentenceStatus (const char *source) : type (source)
        {
            lastReceived = 0;
            count        = 0;
        }
    };

    #pragma pack()

    typedef std::vector <SentenceStatus> SentenceStatusArray;

    class SentenceRegistry : public std::map <unsigned int, SentenceStatus>
    {
        public:
            void update (const char *sentenceName);

            SentenceStatus *findByName (const char *sentenceName);

            void populate (SentenceStatusArray&);
    };
}
