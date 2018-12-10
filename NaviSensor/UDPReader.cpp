#include "UDPReader.h"

Readers::UDPReader::UDPReader (Sensors::UdpParams *config) : Reader (config)
{
    socket = 0;
}

Readers::UDPReader::~UDPReader ()
{
    if (socket)
        delete socket;
}

size_t Readers::UDPReader::read ()
{
    int totalBytesRead = 0;

    if (socket)
    {
        unsigned long bytesAvailable;
        unsigned char buffer [0xFFFF];
        in_addr       sender;
        int           bytesRead;

        while (bytesAvailable  = socket->getAvalDataSize (), bytesAvailable > 0)
        {
            if (bytesAvailable > sizeof (buffer))
                bytesAvailable = sizeof (buffer);

            bytesRead = socket->receiveFrom ((char *) buffer, bytesAvailable, sender);

            if (bytesRead > 0)
                queue.pushBuffer (buffer, bytesRead);

            totalBytesRead += bytesRead;

            Tools::sleepFor (5);
        }
    }

    return totalBytesRead;
}

bool Readers::UDPReader::open ()
{
    Sensors::UdpParams *params = (Sensors::UdpParams *) config;
    bool                result = false;

    if (socket)
    {
        socket->close ();

        delete socket;
    }

    socket = new Comm::Socket ();

    result = socket->create (params->inPort, false, params->bind);
    opened = result;

    return opened;
}

void Readers::UDPReader::close ()
{
    if (opened)
    {
        if (socket)
        {
            if (socket->opened())
                socket->close ();

            delete socket;

            socket = 0;
        }

        opened = false;
    }
}
