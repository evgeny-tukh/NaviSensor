#include "DataStorage.h"
#include "../NaviSensorUI//tools.h"

Data::SensorDataStorage::SensorDataStorage (const int paramTimeout) : watchdog (watchdogProcInternal, this)
{
    this->paramTimeout = paramTimeout;
    this->active       = true;
}

Data::SensorDataStorage::~SensorDataStorage ()
{
    active = false;

    watchdog.join ();
}

void Data::SensorDataStorage::update (Parameter& param)
{
    iterator pos = find (param.type);

    if (pos == end ())
        pos = insert (end (), std::pair <DataType, Parameter *> (param.type, new Parameter));

    pos->second->assign (param);
}

Data::Parameter *Data::SensorDataStorage::findParam (Data::DataType type)
{
    iterator pos = find (type);

    return pos == end () ? 0 : pos->second;
}

void Data::SensorDataStorage::extractAll (Data::ParamArray& params)
{
    params.clear ();

    for (iterator iter = begin (); iter != end (); ++iter)
        params.push_back (iter->second);
}

void Data::SensorDataStorage::watchdogProc ()
{
    time_t now = time (0);

    while (active)
    {
        for (iterator iter = begin (); iter != end (); ++ iter)
        {
            Parameter *param = iter->second;

            if ((now - param->updateTime) > paramTimeout)
                param->quality = Quality::Poor;
        }

        Tools::sleepFor (1000);
    }
}

void Data::SensorDataStorage::watchdogProcInternal (Data::SensorDataStorage *self)
{
    if (self)
        self->watchdogProc ();
}

Data::GlobalDataStorage::GlobalDataStorage (const int paramTimeout) : watchdog (watchdogProcInternal, this)
{
    this->paramTimeout = paramTimeout;
    this->active       = true;
}

Data::GlobalDataStorage::~GlobalDataStorage ()
{
    active = false;

    watchdog.join ();
}

void Data::GlobalDataStorage::update (const int sensorID, Parameter& param)
{
    iterator typePos = find (param.type);

    if (typePos == end ())
        typePos = insert(end(), std::pair <DataType, ParamMap *> (param.type, new ParamMap));

    ParamMap *params = typePos->second;

    ParamMap::iterator paramPos = params->find (sensorID);

    if (paramPos == params->end ())
        paramPos = params->insert (params->end (), std::pair <int, Parameter *> (sensorID, new Parameter));

    paramPos->second->assign (param);
}

Data::Parameter *Data::GlobalDataStorage::findFirst (Data::DataType type, int& sensorID, const bool goodOnly)
{
    Data::Parameter *result  = 0;
    iterator         typePos = find (type);

    if (typePos != end ())
    {
        ParamMap *params = typePos->second;

        for (ParamMap::iterator iter = params->begin(); iter != params->end(); ++iter)
        {
            if (!goodOnly || iter->second->quality == Quality::Good)
            {
                result   = iter->second;
                sensorID = iter->first;

                break;
            }
        }
    }

    return result;
}

void Data::GlobalDataStorage::extractAll (Data::ParamArray& params, Data::DataType type, const bool goodOnly)
{
    iterator typePos = find(type);

    params.clear ();

    if (typePos != end ())
    {
        ParamMap *paramMap = typePos->second;

        for (ParamMap::iterator iter = paramMap->begin(); iter != paramMap->end (); ++ iter)
        {
            if (!goodOnly || iter->second->quality == Quality::Good)
                params.push_back (iter->second);
        }
    }
}

void Data::GlobalDataStorage::watchdogProc ()
{
    time_t now = time (0);

    while (active)
    {
        for (iterator typeIter = begin (); typeIter != end(); ++ typeIter)
        {
            ParamMap *params = typeIter->second;

            for (ParamMap::iterator paramIter = params->begin(); paramIter != params->end (); ++ paramIter)
            {
                Parameter *param = paramIter->second;

                if ((now - param->updateTime) > paramTimeout)
                    param->quality = Quality::Poor;
            }
        }

        Tools::sleepFor (1000);
    }
}

void Data::GlobalDataStorage::watchdogProcInternal (Data::GlobalDataStorage *self)
{
    if (self)
        self->watchdogProc ();
}
