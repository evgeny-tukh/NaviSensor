#pragma once

#include <queue>
#include <string>

namespace AIS
{
    class BitContainer
    {
        public:
            BitContainer ();

            void push_back (const unsigned char);
            void pop_front ();
            const unsigned char front ();
            void clear ();

            const size_t size ();

            void saveState ();
            void restoreState ();

        private:
            size_t        frontPtr, backPtr, saveFrontPtr, saveBackPtr;
            unsigned char data [0xFFFF];
    };

    class SixBitStorage
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

            inline void clear () { if (container.size () > 0) container.clear (); }

            inline void saveState () { container.saveState (); }
            inline void restoreState () { container.restoreState (); }

        private:
            typedef std::deque <unsigned char> Container;

            BitContainer container;

            void addChar (const char);

            void setBit (const int bitIndex, unsigned char *buffer, const unsigned char bitValue);
    };

    unsigned char asciiToSixBit (unsigned char ascii);
}
