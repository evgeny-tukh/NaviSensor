#include "Reader.h"

Readers::Reader::Reader (Sensors::Config *config)
{
    opened = false;

    this->config = config;
}

Readers::Reader::~Reader ()
{
    close ();
}

size_t Readers::Reader::read ()
{
    return 0;
}

bool Readers::Reader::open ()
{
    opened = true;

    return opened;
}

void Readers::Reader::close()
{
    opened = false;
}

size_t Readers::Reader::getData (byte *buffer, const size_t size)
{
    return queue.pull (buffer, size);
}

size_t Readers::Reader::getData (char *buffer, const size_t size, const char *eol)
{
    return queue.pull (buffer, size, "\n\r", "!$");
}
