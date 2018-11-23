#pragma once

#include <WinSock.h>
#include <thread>
#include <vector>

namespace Comm
{
    typedef std::vector <std::thread *> ThreadPool;

    typedef std::function <int (byte *, size_t)> CommCallback;

    typedef std::function <void (CommCallback sendCb, CommCallback rcvCb, void *data)> CommLogicProc;

    enum Command
    {
        Ack          = 99,
        Nak          = 100,

        StartAll     = 1,
        StopAll      = 2,
        StartInputFw = 3,
        StopInputFw  = 4
    };

    class DataNode
    {
        public:
            DataNode (CommLogicProc logicProc, void *logicParam = 0, const int port = 8000, const char *bindAddr = "127.0.0.1");
            virtual ~DataNode ();

            virtual bool create ();
            virtual void close ();

            static void init ();

        protected:
            CommLogicProc logicProc;
            SOCKET        handle;
            bool          active;
            sockaddr_in   localAddr;
            void         *logicParam;
    };

    class DataServer : public DataNode
    {
        public:
            DataServer (CommLogicProc logicProc, void *logicParam = 0, const int port = 8000, const char *bindAddr = "127.0.0.1");
            virtual ~DataServer () {}

            virtual void close ();

        protected:
            std::thread listener;
            ThreadPool  connections;

            void workerProc ();
            static void workerProcInternal (DataServer *);

            static void connectionProcInternal (SOCKET, DataServer *);
            void connectionProc (SOCKET);
    };

    class DataClient : public DataNode
    {
        public:
            DataClient (CommLogicProc logicProc, void *logicParam = 0, const int port = 8000, const char *bindAddr = "127.0.0.1");
            virtual ~DataClient () {}

            virtual void close ();

            bool connectTo (const char *host = "127.0.0.1", const int port = 8000);

            int sendCommand (const Command command);

            bool waitForAck ();

        protected:
            std::thread *worker;

            void workerProc ();
            static void workerProcInternal (DataClient *);
    };
}