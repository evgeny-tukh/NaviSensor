#include "LocalListener.h"
#include "tools.h"

Comm::LocalListener::LocalListener (unsigned int port, bool *active, Comm::Socket::ReadCb callback) : std::thread (procInternal, this)
{
    this->port     = port;
    this->active   = active;
    this->callback = callback;
}

void Comm::LocalListener::procInternal (LocalListener *self)
{
    self->proc ();
}

void Comm::LocalListener::proc()
{
    Comm::Socket     receiver;
    char             buffer[10000];
    in_addr          sender, localhost;
    int              bytesRead;
    Tools::AddrArray interfaces;

    Tools::getInterfaceList (interfaces);

    localhost.S_un.S_addr = htonl (INADDR_LOOPBACK);

    interfaces.push_back (localhost);

    receiver.create (port);

    while (*active)
    {
        unsigned long dataAvailable = receiver.getAvalDataSize ();

        if (dataAvailable > 0)
        {
            if (dataAvailable > sizeof (buffer))
                dataAvailable = sizeof (buffer);

            bytesRead = receiver.receiveFrom (buffer, dataAvailable, sender);

            if (bytesRead > 0)
            {
                bool isLocal = false;

                for (auto & addr : interfaces)
                {
                    if (addr.S_un.S_addr == sender.S_un.S_addr)
                    {
                        isLocal = true; break;
                    }
                }

                if (isLocal)
                    callback (buffer, bytesRead);
            }
        }

        Tools::sleepFor (5);
    }
}
