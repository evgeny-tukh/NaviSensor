#include "DataStorage.h"
#include "../NaviSensorUI//tools.h"

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
