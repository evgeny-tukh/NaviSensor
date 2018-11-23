#include "DataServer.h"
#include "../NaviSensorUI//tools.h"

void Comm::DataNode::init ()
{
    static bool initialized = false;

    if (!initialized)
    {
        WSADATA data;

        memset (& data, 0, sizeof (data));

        WSAStartup (0x0101, & data);

        initialized = true;
    }
}

Comm::DataNode::DataNode (CommLogicProc logicProc, void *logicParam, const int port, const char *bindAddr) : active (true)
{
    localAddr.sin_addr.S_un.S_addr = inet_addr (bindAddr);
    localAddr.sin_family           = AF_INET;
    localAddr.sin_port             = htons (port);

    handle = INVALID_SOCKET;
    active = false;

    this->logicProc  = logicProc;
    this->logicParam = logicParam;
}

Comm::DataNode::~DataNode ()
{
    close ();
}

bool Comm::DataNode::create ()
{
    bool result;

    handle = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    result = handle != INVALID_SOCKET;

    if (result && bind (handle, (sockaddr *) & localAddr, sizeof (localAddr)) != S_OK)
        result = false;
    else
        active = true;

    return result;
}

void Comm::DataNode::close ()
{
    active = false;

    if (handle != INVALID_SOCKET)
    {
        closesocket (handle);

        handle = INVALID_SOCKET;
    }
}

Comm::DataServer::DataServer (CommLogicProc logicProc, void *logicParam, const int port, const char *bindAddr) :
    DataNode (logicProc, logicParam, port, bindAddr), listener (Comm::DataServer::workerProcInternal, this)
{
    this->logicProc = logicProc;
}

void Comm::DataServer::close ()
{
    if (active && listener.joinable ())
    {
        active = false;

        listener.join ();
    }

    DataNode::close();
}

void Comm::DataServer::workerProc ()
{
    while (active)
    {
        if (handle != INVALID_SOCKET)
        {
            if (listen (handle, SOMAXCONN) == S_OK)
            {
                sockaddr_in remoteAddr;
                int         size         = sizeof(remoteAddr);
                SOCKET      incomingConn = accept (handle, (sockaddr *) & remoteAddr, & size);

                if (incomingConn != INVALID_SOCKET)
                {
                    std::thread *pipe = new std::thread (Comm::DataServer::connectionProcInternal, incomingConn, this);

                    connections.push_back (pipe);
                }
            }
        }

        Tools::sleepFor (100);
    }
}

void Comm::DataServer::connectionProcInternal (SOCKET connection, DataServer *self)
{
    if (self)
        self->connectionProc (connection);
}

void Comm::DataServer::connectionProc (SOCKET connection)
{
    std::thread::id id = std::this_thread::get_id ();

    logicProc ([connection] (byte *buffer, size_t size) -> int
               {
                   return send (connection, (const char *) buffer, (int) size, 0);
               },
               [connection] (byte *buffer, size_t size) -> int
               {
                   return recv (connection, (char *) buffer, (int) size, 0);
               },
               logicParam);

    closesocket (connection);

    for (ThreadPool::iterator iter = connections.begin (); iter != connections.end (); ++ iter)
    {
        std::thread *curThread = *iter;

        if (curThread->get_id () == id)
        {
            curThread->detach ();

            delete curThread;

            connections.erase (iter); break;
        }
    }
}

void Comm::DataServer::workerProcInternal (DataServer *self)
{
    if (self)
        self->workerProc ();
}

Comm::DataClient::DataClient (CommLogicProc logicProc, void *logicParam, const int port, const char *bindAddr) :
    DataNode (logicProc, logicParam, port, bindAddr), worker (0)
{
}

void Comm::DataClient::close ()
{
    if (active && worker && worker->joinable ())
    {
        active = false;

        worker->join ();
    }

    DataNode::close ();
}

bool Comm::DataClient::connectTo (const char *host, const int port)
{
    sockaddr_in remote;
    bool        result = false;

    remote.sin_addr.S_un.S_addr = inet_addr (host);
    remote.sin_family           = AF_INET;
    remote.sin_port             = htons (port);

    if (connect (handle, (sockaddr *) & remote, sizeof (remote)) == S_OK)
    {
        worker = new std::thread (workerProcInternal, this); result = true;
    }

    return result;
}

void Comm::DataClient::workerProcInternal (DataClient *self)
{
    if (self)
        self->workerProc ();
}

void Comm::DataClient::workerProc ()
{
    logicProc ([this] (byte *buffer, size_t size) -> int
               {
                   return send (handle, (const char *) buffer, (int) size, 0);
               },
               [this] (byte *buffer, size_t size) -> int
               {
                   return recv (handle, (char *) buffer, (int) size, 0);
               },
               logicParam);
}
