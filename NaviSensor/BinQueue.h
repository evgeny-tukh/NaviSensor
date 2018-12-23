#pragma once

#include <vector>
#include <queue>
#include <mutex>

namespace Readers
{
    typedef unsigned char byte;
    typedef std::vector <byte> ByteBuffer;

    class BinaryQueue
    {
        public:
            BinaryQueue ();

            void pushBuffer (ByteBuffer& data);
            void pushBuffer (const byte *data, const size_t size);
            size_t pull (byte *buffer, const size_t size, const bool needLock = true);
            size_t pull (char *buffer, const size_t size, const char *finishAfterChars, const char *finishBeforeChars, const bool ignoreUnfinished = true);

        protected:
            typedef public std::deque <byte> Container;

            Container  container;
            std::mutex locker;
    };
}