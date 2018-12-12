#include "SixBitStorage.h"

#define _101000B_           0x28
#define _110000B_           0x30
#define _111000B_           0x38
#define _100000B_           0x20
#define _10000000B_         0x80

void AIS::SixBitStorage::add (const char *data)
{
    for (char *curPtr = (char *) data; *curPtr; addChar (*(curPtr ++ )));
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
        push ((byte & mask) ? 1 : 0);
}

unsigned int AIS::SixBitStorage::getData (const int numOfBits)
{
    unsigned int data  = 0,
                 start = 8 - (numOfBits % 8);

    for (int i = 0; i < numOfBits; ++ i)
    {
        const unsigned char bitValue = front ();

        setBit (i + start, (unsigned char *) & data, bitValue);

        pop ();
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

unsigned char AIS::asciiToSixBit (unsigned char ascii)
{
    ascii += _101000B_;

    return (ascii > _10000000B_) ? ((ascii + _100000B_) & 0x3F) : ((ascii + _101000B_) & 0x3F);
}
