#include "SixBitStorage.h"

#define _101000B_           0x28
#define _110000B_           0x30
#define _111000B_           0x38
#define _100000B_           0x20
#define _10000000B_         0x80

AIS::BitContainer::BitContainer ()
{
    frontPtr = backPtr = 0;

    memset (data, 0, sizeof (data));
}

void AIS::BitContainer::saveState ()
{
    saveFrontPtr = frontPtr;
    saveBackPtr  = backPtr;
}

void AIS::BitContainer::restoreState ()
{
    frontPtr = saveFrontPtr;
    backPtr  = saveBackPtr;
}

void AIS::BitContainer::clear ()
{
    frontPtr = backPtr = 0;
}

void AIS::BitContainer::push_back (const unsigned char newByte)
{
    if (backPtr < sizeof (data))
        data [backPtr++] = newByte;
}

void AIS::BitContainer::pop_front ()
{
    if (frontPtr < backPtr && frontPtr < sizeof (data))
        ++ frontPtr;
}

const unsigned char AIS::BitContainer::front ()
{
    return (frontPtr < backPtr) ? data [frontPtr] : 0;
}

const size_t AIS::BitContainer::size()
{
    return backPtr - frontPtr;
}

void AIS::SixBitStorage::add (const char *data)
{
    for (char *curPtr = (char *) data; *curPtr; addChar (*(curPtr ++ )));
}

bool AIS::SixBitStorage::getFlag ()
{
    return getByte (1) != 0;
}

unsigned char AIS::SixBitStorage::getByte (const int numOfBits)
{
    return (unsigned char) getData (numOfBits);
}

unsigned short AIS::SixBitStorage::getShort (const int numOfBits, const bool signedValue, const bool lsbFirst)
{
    unsigned short value = (unsigned short) getData (numOfBits);

    value = lsbFirst ? ((value & 0xFFFF) << 8) | (value >> 8) : value;

    if (signedValue)
    {
        unsigned short signBitNo    = numOfBits - 1,
                       negativeMask = 1 << signBitNo,
                       unsignedValue;

        if (value & negativeMask)
        {
            unsigned short clearMask, negativeValue, i;

            for (i = 0, clearMask = 1, negativeValue = 0xFFFF; i < signBitNo; ++i, clearMask <<= 1)
                negativeValue -= clearMask;

            unsignedValue = negativeValue | value;
        }
        else
        {
            unsignedValue = value;
        }

        value = unsignedValue;
    }

    return value;
}

unsigned int AIS::SixBitStorage::getInt (const int numOfBits, const bool signedValue, const bool lsbFirst)
{
    #pragma pack(1)
    union
    {
        unsigned int  value;
        unsigned char bytes [4];
    }
    data1, data2;
    #pragma pack()

    data1.value = getData (numOfBits);

    if (lsbFirst)
    {
        for (int i = 0; i < 4; ++ i)
            data2.bytes [3-i] = data1.bytes [i];
    }
    else
    {
        data2.value = data1.value;
    }

    if (signedValue)
    {
        unsigned int signBitNo    = numOfBits - 1,
                     negativeMask = 1 << signBitNo,
                     unsignedValue;

        if (data2.value & negativeMask)
        {
            unsigned int clearMask, negativeValue, i;

            for (i = 0, clearMask = 1, negativeValue = 0xFFFFFFFF; i < signBitNo; ++ i, clearMask <<= 1)
                negativeValue -= clearMask;

            unsignedValue = negativeValue | data2.value;
        }
        else
        {
            unsignedValue = data2.value;
        }

        data2.value = unsignedValue;
    }

    return data2.value;
}

char *AIS::SixBitStorage::getString (char *buffer, const size_t size)
{
    if (buffer)
    {
        memset (buffer, 0, size);
    }

    return buffer;
}

void AIS::SixBitStorage::addChar (const char newChar)
{
    unsigned char byte = asciiToSixBit ((unsigned char) newChar),
                  mask = 0x20;

    for (int i = 0; i < 6; ++ i, mask >>= 1)
        container.push_back ((byte & mask) ? 1 : 0);
}

unsigned int AIS::SixBitStorage::getData (const int numOfBits)
{
    unsigned int data  = 0,
                 start = 8 - (numOfBits % 8);

    for (int i = 0; i < numOfBits; ++ i)
    {
        if (container.size () > 0)
        {
            unsigned char bitValue = container.front ();

            setBit (i + start, (unsigned char *) & data, bitValue);

            container.pop_front ();
        }
        else
        {
            int iii=0;
            ++iii;
        }
    }

    return data;
}

void AIS::SixBitStorage::setBit (const int bitIndex, unsigned char *buffer, const unsigned char bitValue)
{
    int           byteNo = bitIndex >> 3;
    unsigned char mask   = 0x80 >> (bitIndex % 8);

    if (bitValue)
        buffer [byteNo] |= mask;
    else
        buffer [byteNo] &= (0xFF - mask);
}

std::string AIS::SixBitStorage::getString (const int numOfChars)
{
    std::string result;
    char        character [2] = { 0, 0 };

    for (int i = 0; i < numOfChars; ++i)
    {
        unsigned char byte = getByte (6);

        if (byte && (byte < 0x20))
            character [0] = (char) (byte + 0x40);
        else
            character [0] = (char) byte;

        result += character;
    }

    return result;
}

unsigned char AIS::asciiToSixBit(unsigned char ascii)
{
    ascii += _101000B_;

    return (ascii > _10000000B_) ? ((ascii + _100000B_) & 0x3F) : ((ascii + _101000B_) & 0x3F);
}
