#include "AISConfig.h"

#define SECTION_AIS_FILTERING   "AISFiltering"

char AIS::Filtering::defCfgPath [1000] = { "" };

void AIS::Filtering::assign (AIS::Filtering& source)
{
    limitAmount = source.limitAmount;
    limitRange  = source.limitRange;
    maxAmount   = source.maxAmount;
    maxRange    = source.maxRange;
}

void AIS::Filtering::save()
{
    saveData (SECTION_AIS_FILTERING, "LimitAmount", limitAmount);
    saveData (SECTION_AIS_FILTERING, "LimitRange", limitRange);
    saveData (SECTION_AIS_FILTERING, "MaxAmount", maxAmount);
    saveData (SECTION_AIS_FILTERING, "MaxRange", maxRange);
}

void AIS::Filtering::load()
{
    limitAmount = loadNumericData (SECTION_AIS_FILTERING, "LimitAmount", 1) != 0;
    limitRange  = loadNumericData (SECTION_AIS_FILTERING, "LimitRange", 1) != 0;
    maxAmount   = loadNumericData (SECTION_AIS_FILTERING, "MaxAmount", 150);
    maxRange    = loadNumericData (SECTION_AIS_FILTERING, "MaxRange", 25);
}
