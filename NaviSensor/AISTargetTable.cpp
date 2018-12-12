#include "AISTargetTable.h"
#include "../NaviSensorUI/tools.h"

AIS::AISTargetTable::AISTargetTable (const time_t timeout) : active (true), watchdog (watchdogProcInternal, this)
{
    this->timeout = timeout;
}

AIS::AISTargetTable::~AISTargetTable()
{
    active = false;

    if (watchdog.joinable ())
        watchdog.join ();

    for (auto & item : *this)
    {
        if (item.second.second)
            delete item.second.second;
    }
}

AIS::AISTarget *AIS::AISTargetTable::findTarget (const unsigned int mmsi)
{
    auto pos = find (mmsi);

    return pos == end () ? 0 : pos->second.second;
}

AIS::AISTarget *AIS::AISTargetTable::checkAddTarget (const unsigned int mmsi)
{
    AIS::AISTarget *target = findTarget (mmsi);

    if (!target)
    {
        target = new AISTarget (mmsi);

        insert (end (), std::pair <unsigned int, AISTargetRec> (mmsi, std::pair <time_t, AISTarget *> (time (0), target)));
    }

    return target;
}

void AIS::AISTargetTable::watchdogProc ()
{
    while (active)
    {
        lock ();

        time_t now = time (0);

        for (iterator pos = begin (); pos != end ();)
        {
            if ((now - pos->second.first) > timeout)
            {
                if (pos->second.second)
                    delete pos->second.second;

                pos = erase (pos);
            }
            else
            {
                ++ pos;
            }
        }

        unlock ();

        Tools::sleepFor (5000);
    }
}

void AIS::AISTargetTable::watchdogProcInternal (AIS::AISTargetTable *self)
{
    if (self)
        self->watchdogProc ();
}
