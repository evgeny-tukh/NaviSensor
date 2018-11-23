#pragma once

#include <vector>
#include <mutex>

namespace Readers
{
    typedef unsigned char byte;
    typedef std::vector <byte> ByteBuffer;

    class BinaryQueue : public ByteBuffer
    {
        public:
            BinaryQueue ();

            void push (ByteBuffer& data);
            void push (const byte *data, const size_t size);
            size_t pull (byte *buffer, const size_t size, const bool needLock = true);
            size_t pull (char *buffer, const size_t size, const char *eol);

        protected:
            std::mutex locker;
    };
}