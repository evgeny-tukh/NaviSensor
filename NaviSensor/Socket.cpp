#include <thread>
#include "Socket.h"
#include "../NaviSensorUI//tools.h"

#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374

Comm::Socket::Socket ()
{
    handle = INVALID_SOCKET;
}

Comm::Socket::~Socket ()
{
    close ();
}

bool Comm::Socket::createTcp (const char *bindAddr)
{
    in_addr local;

    local.S_un.S_addr = bindAddr? inet_addr (bindAddr) : 0;

    return createTcp (local);
}

bool Comm::Socket::createTcp (const in_addr bindAddr)
{
    SOCKADDR_IN local;

    handle = socket(AF_INET, SOCK_STREAM, 0);

    local.sin_family           = AF_INET;
    local.sin_addr.S_un.S_addr = bindAddr.S_un.S_addr;
    local.sin_port             = 0;

    return bind (handle, (sockaddr *) & local, sizeof (local)) == S_OK;
}

bool Comm::Socket::connectTo (const unsigned int port, const char *remoteAddr)
{
    in_addr remote;

    remote.S_un.S_addr = inet_addr (remoteAddr);

    return connectTo (port, remote);
}

bool Comm::Socket::connectTo (const unsigned int port, const in_addr remoteAddr)
{
    SOCKADDR_IN remote;

    remote.sin_family           = AF_INET;
    remote.sin_addr.S_un.S_addr = remoteAddr.S_un.S_addr;
    remote.sin_port             = htons (port);

    return connect (handle, (sockaddr *) & remote, sizeof (remote)) == S_OK;
}


bool Comm::Socket::create (const unsigned int port, const bool broadcast, const in_addr bindAddr)
{
    bool result = false;

    handle = socket (AF_INET, SOCK_DGRAM, 0/*IPPROTO_IP*/);

    if (handle != INVALID_SOCKET)
    {
        sockaddr_in  local;
        unsigned int trueVal = 1;

        setsockopt (handle, SOL_SOCKET, SO_REUSEADDR, (const char *) & trueVal, sizeof (trueVal));

        memset (&local, 0, sizeof (local));

        local.sin_family           = AF_INET;
        local.sin_port             = htons(port);
        local.sin_addr.S_un.S_addr = bindAddr.S_un.S_addr;

        if (bind (handle, (sockaddr *) & local, sizeof (local)) == 0)
            result = true;

        if (broadcast)
            setsockopt (handle, SOL_SOCKET, SO_BROADCAST, (const char *) & trueVal, sizeof (trueVal));
    }

    return result;
}

bool Comm::Socket::create (const unsigned int port, const bool broadcast, const char *bindAddr)
{
    bool result = false;

    handle = socket (AF_INET, SOCK_DGRAM, 0/*IPPROTO_IP*/);

    if (handle != INVALID_SOCKET)
    {
        sockaddr_in  local;
        unsigned int trueVal = 1;

        setsockopt (handle, SOL_SOCKET, SO_REUSEADDR, (const char *) & trueVal, sizeof (trueVal));

        memset (& local, 0, sizeof (local));

        local.sin_family           = AF_INET;
        local.sin_port             = htons (port);
        local.sin_addr.S_un.S_addr = bindAddr ? inet_addr (bindAddr) : htonl (INADDR_ANY);

        if (bind (handle, (sockaddr *) & local, sizeof (local)) == 0)
            result = true;

        //if (result && WSAAsyncSelect (handle, 0, WM_SOCKET_NOTIFY, 0) != 0)
        //    result = false;

        //if (ioctlsocket (handle, FIONBIO, (unsigned long *) & trueVal) != 0)
        //    result = false;

        if (broadcast)
            setsockopt (handle, SOL_SOCKET, SO_BROADCAST, (const char *) & trueVal, sizeof (trueVal));
    }

    return result;
}

void Comm::Socket::close ()
{
    if (handle != INVALID_SOCKET)
    {
        closesocket (handle);

        handle = INVALID_SOCKET;
    }
}

int Comm::Socket::sendTo (const char *data, const int size, const int port, const char *destAddr)
{
    sockaddr_in dest;

    dest.sin_addr.S_un.S_addr = destAddr ? inet_addr (destAddr) : htonl (INADDR_BROADCAST);
    dest.sin_family           = AF_INET;
    dest.sin_port             = htons (port);

    return sendto (handle, data, size > 0 ? size : strlen (data), 0, (sockaddr *) & dest, sizeof (dest));
}

int Comm::Socket::receive (char *buffer, const int size)
{
    return buffer && size > 0 ? recv (handle, buffer, size, 0) : 0;
}

int Comm::Socket::receiveFrom (char *buffer, const int size, in_addr& senderAddr)
{
    sockaddr_in origin;
    int         originSize = sizeof (origin);
    int         received;
    
    memset (& origin, 0, sizeof (origin));

    received = recvfrom (handle, buffer, size, 0, (sockaddr *) & origin, & originSize);

    if (received > 0)
        senderAddr.S_un.S_addr = origin.sin_addr.S_un.S_addr;

    return received;
}

int Comm::Socket::receiveFrom (char *buffer, const int size, char *senderAddr, const int addrSize)
{
    in_addr sender;
    int     result = receiveFrom (buffer, size, sender);

    if (senderAddr && result > 0)
        strncpy (senderAddr, inet_ntoa (sender), addrSize);

    return result;
}

void Comm::Socket::listen (ReadCb callback, bool *finish, void *param, const char *onlyFrom)
{
    std::thread listener ([this] (ReadCb callback, bool *finish, void *param, const char *onlyFrom) -> void
                          {
                              unsigned long bytesAvailable;
                              in_addr       onlyFromAddr;
                              char          buffer [0xFFFF];
                              
                              onlyFromAddr.S_un.S_addr = onlyFrom ? inet_addr (onlyFrom) : htonl (INADDR_ANY);

                              while (!(*finish))
                              {
                                  if (ioctlsocket (handle, FIONREAD, &bytesAvailable) == 0 && bytesAvailable > 0)
                                  {
                                      in_addr sender;
                                      int     bytesRead = this->receiveFrom (buffer, bytesAvailable, sender);

                                      if (bytesRead > 0 && (onlyFromAddr.S_un.S_addr == 0 || onlyFromAddr.S_un.S_addr == sender.S_un.S_addr))
                                          callback (buffer, bytesRead);
                                  }

                                  Tools::sleepFor (5);
                              }
                          },
                          callback, finish, param, onlyFrom);

    listener.detach ();
}

void Comm::Socket::readAll (ReadCb callback, bool *finish, void *param, const char *onlyFrom)
{
    unsigned long bytesAvailable;
    in_addr       onlyFromAddr;
    char          buffer [10000];

    onlyFromAddr.S_un.S_addr = onlyFrom ? inet_addr (onlyFrom) : htonl (INADDR_ANY);

    while (!(*finish) && ioctlsocket (handle, FIONREAD, &bytesAvailable) == 0 && bytesAvailable > 0)
    {
        in_addr sender;
        int     bytesRead = this->receiveFrom (buffer, /*bytesAvailable*/sizeof (buffer), sender);

        if (bytesRead > 0 && (onlyFromAddr.S_un.S_addr == 0 || onlyFromAddr.S_un.S_addr == sender.S_un.S_addr))
            callback(buffer, bytesRead);

        Tools::sleepFor (5);
    }
}

unsigned long Comm::Socket::getAvalDataSize ()
{
    unsigned long bytesAvailable;

    return ioctlsocket (handle, FIONREAD, & bytesAvailable) == 0 ? bytesAvailable : 0;
}
