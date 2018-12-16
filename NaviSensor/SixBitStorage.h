#pragma once

#include <queue>
#include <string>

namespace AIS
{
    class SixBitStorage : public std::queue <unsigned char>
    {
        public:
            void add (const char *);

            unsigned char getByte (const int numOfBits);
            unsigned short getShort (const int numOfBits, const bool signedValue = false, const bool lsbFirst = true);
            unsigned int getInt (const int numOfBits, const bool signedValue = false, const bool lsbFirst = true);

            bool getFlag ();

            std::string getString (const int numOfChars);

            unsigned int getData (const int numOfBits);

            char *getString (char *buffer, const size_t size);

            inline void clear () { c.clear (); }

        private:
            void addChar (const char);

            void setBit (const int bitIndex, unsigned char *buffer, const unsigned char bitValue);
    };

    unsigned char asciiToSixBit (unsigned char ascii);
}
