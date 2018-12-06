#pragma once

#include <WinSock.h>
#include <functional>

namespace Comm
{
    class Socket
    {
        public:
            typedef std::function <void (char *, const int)> ReadCb;

            Socket ();
            virtual ~Socket ();

            bool create (const unsigned int port = 0, const bool broadcast = false, const char *bindAddr = 0);
            void close ();

            int sendTo (const char *data, const int size, const int port, const char *dest = 0);
            int receiveFrom (char *buffer, const int size, in_addr& senderAddr);
            int receiveFrom (char *buffer, const int size, char *senderAddr, const int addrSize);

            void readAll (ReadCb callback, bool *finish, void *param = 0, const char *onlyFrom = 0);
            void listen (ReadCb callback, bool *finish, void *param = 0, const char *onlyFrom = 0);

            unsigned long getAvalDataSize ();

        protected:
            SOCKET handle;
    };
}