#include "TCPReader.h"

Readers::TCPReader::TCPReader (Sensors::TcpParams *config) : UDPReader (config)
{
}

Readers::TCPReader::~TCPReader ()
{
}

size_t Readers::TCPReader::read ()
{
    int totalBytesRead = 0;

    if (socket)
    {
        unsigned long bytesAvailable;
        unsigned char buffer [0xFFFF];
        int           bytesRead;

        while (bytesAvailable = socket->getAvalDataSize(), bytesAvailable > 0)
        {
            if (bytesAvailable > sizeof(buffer))
                bytesAvailable = sizeof(buffer);

            bytesRead = socket->receive ((char *) buffer, bytesAvailable);

            if (bytesRead > 0)
                queue.pushBuffer (buffer, bytesRead);

            totalBytesRead += bytesRead;

            Tools::sleepFor (2);
        }
    }

    return totalBytesRead;
}

bool Readers::TCPReader::open ()
{
    Sensors::TcpParams *params = (Sensors::TcpParams *) config;
    bool                result = false;

    if (socket)
    {
        socket->close ();

        delete socket;
    }

    socket = new Comm::Socket();

    return socket->createTcp () && socket->connectTo (params->outPort, params->dest);
}

void Readers::TCPReader::close ()
{
    if (opened)
    {
        if (socket)
        {
            if (socket->opened())
                socket->close();

            delete socket;

            socket = 0;
        }

        opened = false;
    }
}
