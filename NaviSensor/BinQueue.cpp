#include "BinQueue.h"

Readers::BinaryQueue::BinaryQueue ()
{

}

void Readers::BinaryQueue::pushBuffer (ByteBuffer& data)
{
    locker.lock ();
    
    container.insert (container.end (), data.begin (), data.end ());
    //for (ByteBuffer::iterator ref = data.begin (); ref != data.end (); ++ ref)
    //    container.push_back (*ref);

    locker.unlock ();
}

void Readers::BinaryQueue::pushBuffer (const byte *data, const size_t size)
{
    char *begin = (char *) data,
         *end   = begin + size;

    locker.lock ();

    container.insert (container.end (), begin, end);
    //for (size_t i = 0; i < size; ++ i)
    //    container.push_back (data [i]);

    locker.unlock ();
}

size_t Readers::BinaryQueue::pull (byte *buffer, const size_t size, const bool needLock)
{
    size_t actualSize = container.size () >= size ? size : container.size ();

    if (needLock)
        locker.lock ();
    
    memset (buffer, 0, size);

    for (size_t i = 0; i < actualSize; ++ i)
    {
        buffer [i] = container.front ();

        container.pop_front ();
    }

    if (needLock)
        locker.unlock ();

    return actualSize;
}

size_t Readers::BinaryQueue::pull (char *buffer, const size_t bufSize, const char *finishAfterChars, const char *finishBeforeChars, const bool ignoreUnfinished)
{
    size_t actualSize = container.size () >= bufSize ? bufSize : container.size (),
           queueSize  = container.size (),
           start,
           count;
    bool   finishFound,
           noCharsPassed;

    locker.lock ();

    if (container.empty())
    {
        locker.unlock (); return 0;
    }

    //memset (buffer, 0, bufSize);

    for (finishFound = false, count = start = 0, noCharsPassed = true; !finishFound && count < queueSize && (count - start) < bufSize; ++ count)
    {
        char curChar = (char) container [count];

        if (curChar)
        {
            if (strchr (finishAfterChars, curChar) != 0)
            {
                if (noCharsPassed)
                {
                    ++ start;
                }
                else
                {
                    finishFound = true;

                    // Skip all "concluding" chars (like \n after \r)
                    for (size_t index = count + 1; index < container.size () && strchr (finishAfterChars, container [index]) != 0; ++ index)
                        count = index;
                }
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
        size_t lastElement = count - start;

        if (lastElement < bufSize)
            buffer [lastElement] = '\0';

        for (size_t i = 0, j = 0; i < count; ++i)
        {
            if (start == 0)
                buffer [j++] = (char) container.at (i);
            else
                -- start;

            //container.pop_front ();
        }

        container.erase (container.begin (), container.begin () + (count - 1));
    }

    locker.unlock ();

    return count;
}
