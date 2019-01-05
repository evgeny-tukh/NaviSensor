#include "DataStorage.h"
#include "../NaviSensorUI//tools.h"

Data::SensorDataStorage::SensorDataStorage (const int paramTimeout) : active (true), watchdog (watchdogProcInternal, this)
{
    this->paramTimeout = paramTimeout;
}

Data::SensorDataStorage::~SensorDataStorage ()
{
    active = false;

    watchdog.join ();
}

void Data::SensorDataStorage::update (Parameter& param)
{
    locker.lock ();

    iterator pos = find (param.type);

    if (pos == end ())
        pos = insert (end (), std::pair <DataType, Parameter *> (param.type, new Parameter));

    pos->second->assign (param);

    locker.unlock ();
}

Data::Parameter *Data::SensorDataStorage::findParam (Data::DataType type)
{
    iterator pos = find (type);

    return pos == end () ? 0 : pos->second;
}

void Data::SensorDataStorage::extractAll (Data::ParamArray& params)
{
    params.clear ();

    locker.lock ();

    for (iterator iter = begin (); iter != end (); ++iter)
        params.push_back (iter->second);

    locker.unlock ();
}

void Data::SensorDataStorage::watchdogProc ()
{
    time_t now;

    while (active)
    {
        now = time (0);

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

Data::GlobalDataStorage::GlobalDataStorage (const int paramTimeout, Data::Pos *curPosition) : active (true), watchdog (watchdogProcInternal, this)
{
    this->curPosition  = curPosition;
    this->paramTimeout = paramTimeout;
}

Data::GlobalDataStorage::~GlobalDataStorage ()
{
    active = false;

    watchdog.join ();
}

void Data::GlobalDataStorage::update (const int sensorID, Parameter& param)
{
    locker.lock ();

    iterator typePos = find (param.type);

    if (typePos == end ())
        typePos = insert (end (), std::pair <DataType, ParamMap *> (param.type, new ParamMap));

    ParamMap *params = typePos->second;

    // Assign or reassing master source if needed
    if (param.quality == Data::Quality::Good)
    {
        int masterSourceID = params->getMasterSourceID ();

        if (masterSourceID == 0)
        {
            params->assignMasterSourceID (sensorID);
        }
        else if (masterSourceID != sensorID)
        {
            Data::Parameter *master = params->at (masterSourceID);

            if (master && master->quality != Data::Quality::Good)
                params->assignMasterSourceID (sensorID);
        }
    }

    ParamMap::iterator paramPos = params->find (sensorID);

    if (paramPos == params->end ())
        paramPos = params->insert (params->end (), std::pair <int, Parameter *> (sensorID, new Parameter));

    paramPos->second->assign (param);

    if (param.type == Data::DataType::Position)
    {
        Data::Pos *position = (Data::Pos *) & param.data;

        curPosition->lat = position->lat;
        curPosition->lon = position->lon;
    }

    locker.unlock ();
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

void Data::GlobalDataStorage::extractAll (Data::GlobalParamArray& params, Data::DataType type, const bool goodOnly)
{
    auto processParamType = [this, goodOnly](Data::GlobalDataStorage::iterator typePos, Data::GlobalParamArray& params) -> void
                            {
                                ParamMap *paramMap = typePos->second;

                                for (ParamMap::iterator iter = paramMap->begin(); iter != paramMap->end(); ++iter)
                                {
                                    const int        sensorID = iter->first;
                                    Data::Parameter *param    = iter->second;
                                    bool             master   = Data::alwaysSelected (param->type) || sensorID == paramMap->getMasterSourceID ();

                                    if (!goodOnly || param->quality == Quality::Good)
                                        params.push_back (new GlobalParameter (sensorID, master, *param));
                                }
                            };
                    
    locker.lock ();

    params.clear();

    if (type == Data::DataType::All)
    {
        for (iterator typePos = begin (); typePos != end (); ++ typePos)
            processParamType (typePos, params);
    }
    else
    {
        iterator typePos = find (type);

        if (typePos != end ())
            processParamType (typePos, params);
    }

    locker.unlock();
}

void Data::GlobalDataStorage::watchdogProc ()
{
    time_t now;

    while (active)
    {
        now = time (0);

        locker.lock ();

        for (iterator typeIter = begin (); typeIter != end(); ++ typeIter)
        {
            ParamMap *params = typeIter->second;

            for (ParamMap::iterator paramIter = params->begin (); paramIter != params->end (); ++ paramIter)
            {
                Parameter *param = paramIter->second;

                if ((now - param->updateTime) > paramTimeout)
                    param->quality = Quality::Poor;

                // Disregard master source if it is poor
                if (params->getMasterSourceID () == paramIter->first && param->quality != Quality::Good)
                    params->assignMasterSourceID (0);
            }
        }

        locker.unlock ();

        Tools::sleepFor (500);
    }
}

void Data::GlobalDataStorage::watchdogProcInternal (Data::GlobalDataStorage *self)
{
    if (self)
        self->watchdogProc ();
}

void Data::GlobalDataStorage::assignMasterSource (const int sensorID, Data::DataType paramType)
{
    locker.lock ();

    iterator pos = find (paramType);

    if (pos != end ())
        pos->second->assignMasterSourceID (sensorID);

    locker.unlock ();
}