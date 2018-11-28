#include "BinQueue.h"

Readers::BinaryQueue::BinaryQueue ()
{

}

void Readers::BinaryQueue::pushBuffer (ByteBuffer& data)
{
    locker.lock ();
    
    for (ByteBuffer::iterator ref = data.begin (); ref != data.end (); ++ ref)
        push (*ref);

    locker.unlock ();
}

void Readers::BinaryQueue::pushBuffer (const byte *data, const size_t size)
{
    locker.lock ();

    for (size_t i = 0; i < size; ++ i)
        push (data [i]);

    locker.unlock ();
}

size_t Readers::BinaryQueue::pull (byte *buffer, const size_t size, const bool needLock)
{
    size_t actualSize = this->size () >= size ? size : this->size ();

    if (needLock)
        locker.lock ();
    
    memset (buffer, 0, size);

    for (size_t i = 0; i < actualSize; ++ i)
    {
        buffer [i] = front ();

        pop ();
    }

    if (needLock)
        locker.unlock ();

    return actualSize;
}

size_t Readers::BinaryQueue::pull (char *buffer, const size_t bufSize, const char *finishAfterChars, const char *finishBeforeChars, const bool ignoreUnfinished)
{
    size_t actualSize = this->size () >= bufSize ? bufSize : this->size (),
           queueSize  = size (),
           start,
           count;
    bool   finishFound,
           noCharsPassed;

    locker.lock ();

    memset (buffer, 0, bufSize);

    for (finishFound = false, count = start = 0, noCharsPassed = true; !finishFound && count < queueSize && (count - start) < bufSize; ++ count)
    {
        char curChar = (char) c [count];

        if (curChar)
        {
            if (strchr (finishAfterChars, curChar) != 0)
            {
                if (noCharsPassed)
                    ++ start;
                else
                    finishFound = true;
            }
            else if (strchr (finishBeforeChars, curChar) != 0)
            {
                if (noCharsPassed)
                {
                    noCharsPassed = false;
                }
                else
                {
                    -- count;

                    finishFound = true;
                }
            }
            else
            {
                noCharsPassed = false;
            }
        }
    }

    if (count > 0)
    {
        for (size_t i = 0, j = 0; i < count; ++i)
        {
            if (start == 0)
                buffer [j++] = (char) c.at (0);
            else
                -- start;

            pop ();
        }
    }

    locker.unlock ();

    return count;
}
