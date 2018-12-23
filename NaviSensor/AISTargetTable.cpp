#include "DataDef.h"
#include "AISTargetTable.h"
#include "../NaviSensorUI/tools.h"

AIS::AISTargetTable::AISTargetTable (const time_t timeout, Data::Pos *curPosition) : active (true), watchdog (watchdogProcInternal, this)
{
    this->curPosition = curPosition;
    this->timeout     = timeout;

    loadFiltering ();
}

AIS::AISTargetTable::~AISTargetTable()
{
    active = false;

    if (watchdog.joinable ())
        watchdog.join ();

    for (auto & item : container)
    {
        if (item.second.second)
            delete item.second.second;
    }
}

AIS::AISTargetRec *AIS::AISTargetTable::findMostDistantTarget (double *range)
{
    AIS::AISTargetRec  *mostDistant;
    double              maxRange;
    Container::iterator pos;

    if (fabs (curPosition->lat) > 90.0 || fabs(curPosition->lon) > 180.0)
        return 0;

    for (pos = container.begin(), mostDistant = 0, maxRange = 0.0; pos != container.end(); ++ pos)
    {
        const double curRange = Tools::calcDistanceRaftly (curPosition->lat, curPosition->lon, pos->second.second->dynamicData.lat, pos->second.second->dynamicData.lon);

        if (curRange > maxRange)
        {
            maxRange    = curRange;
            mostDistant = & pos->second;
        }
    }

    if (range)
        *range = maxRange;

    return mostDistant;
}

AIS::AISTarget *AIS::AISTargetTable::findTarget (const unsigned int mmsi, AISTargetRec **record)
{
    auto pos = container.find (mmsi);

    if (record)
        *record = pos == container.end () ? 0 : & pos->second;

    return pos == container.end () ? 0 : pos->second.second;
}

AIS::AISTarget *AIS::AISTargetTable::checkAddTarget (const unsigned int mmsi)
{
    AISTargetRec   *record;
    AIS::AISTarget *target = findTarget (mmsi, & record);

    if (target)
    {
        if (record)
            record->first = time (0);
    }
    else
    {
        target = new AISTarget (mmsi);

        container.insert (container.end (), std::pair <unsigned int, AISTargetRec> (mmsi, std::pair <time_t, AISTarget *> (time (0), target)));
    }

    return target;
}

void AIS::AISTargetTable::watchdogProc ()
{
    while (active)
    {
        lock ();

        time_t now = time (0);

        for (Container::iterator pos = container.begin (); pos != container.end ();)
        {
            if ((now - pos->second.first) > timeout)
            {
                if (pos->second.second)
                    delete pos->second.second;

                pos = container.erase (pos);
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

void AIS::AISTargetTable::extractStaticData (AISStaticDataArray& targets)
{
    lock ();

    targets.clear ();

    for (auto & pos : container)
    {
        AISTarget *target = pos.second.second;

        targets.emplace_back (pos.first, target->flags, target->staticData);
    }

    unlock ();
}

void AIS::AISTargetTable::extractDynamicData (AISDynamicDataArray& targets)
{
    lock ();

    targets.clear ();

    for (auto & pos : container)
    {
        AISTarget *target = pos.second.second;

        targets.emplace_back (pos.first, target->flags, target->dynamicData);
    }

    unlock ();
}

void AIS::AISTargetTable::loadFiltering (const char *cfgFileName)
{
    if (!cfgFileName)
        cfgFileName = "settings.cfg";//(Tools::getAppDataFolder("CAIM", "NaviSensor") + "\\settings.cfg").c_str ();

    filtering.setCfgFileName (cfgFileName);
    filtering.load ();
}
