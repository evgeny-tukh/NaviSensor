#include "BinQueue.h"

Readers::BinaryQueue::BinaryQueue ()
{

}

void Readers::BinaryQueue::push (ByteBuffer& data)
{
    locker.lock ();
    insert (end (), data.begin (), data.end ());
    locker.unlock ();
}

void Readers::BinaryQueue::push (const byte *data, const size_t size)
{
    size_t curSize;

    locker.lock ();

    curSize = this->size ();

    insert (end (), size, 0);
    memcpy (this->data () + curSize, data, size);
    locker.unlock ();
}

size_t Readers::BinaryQueue::pull (byte *buffer, const size_t size, const bool needLock)
{
    size_t actualSize = this->size () >= size ? size : this->size ();
    byte  *data;

    if (needLock)
        locker.lock ();
    
    data = this->data ();

    memcpy (buffer, data, actualSize);

    this->erase (begin (), begin () + (actualSize - 1));

    if (needLock)
        locker.unlock ();

    return actualSize;
}

size_t Readers::BinaryQueue::pull (char *buffer, const size_t bufSize, const char *finish)
{
    char  *data,
          *endPos;
    size_t actualSize, skippedBytes;

    locker.lock ();

    memset (buffer, 0, bufSize);

    data = (char *) this-> data ();

    if (data)
    {
        for (skippedBytes = 0; *data == '\r' || *data == '\n'; ++ skippedBytes, ++data);

        endPos = strstr (data, finish);

        if (!endPos)
            endPos = data + size ();

        actualSize = endPos - data;

        if (actualSize > bufSize)
            actualSize = bufSize;

        memcpy (buffer, data, actualSize);

        if (endPos)
            actualSize += strlen (finish);

        if (begin () != end ())
        {
            size_t sizeToErase = skippedBytes + actualSize;

            if (sizeToErase > this->size ())
                sizeToErase = this->size ();
                
            if (sizeToErase > 0)
                erase (begin (), begin () + sizeToErase);
        }
    }
    else
    {
        actualSize = 0;
    }

    locker.unlock ();

    return actualSize;
}
