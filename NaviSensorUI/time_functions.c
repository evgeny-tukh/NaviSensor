#include "time_functions.h"

#define EPOCH_BIAS  116444736000000000i64

void time_tToFileTime (const time_t timestamp, FILETIME *fileTime)
{
    if (fileTime != NULL)
    {
        LONGLONG time;
        
        _tzset ();
        
        time = Int32x32To64 (timestamp - _timezone, 10000000) + EPOCH_BIAS;
        
        fileTime->dwLowDateTime  = (DWORD) time;
        fileTime->dwHighDateTime = (DWORD) (time >> 32);
    }
}

void time_tToLocalFileTime (const time_t timestamp, FILETIME *fileTime)
{
    if (fileTime != NULL)
    {
        LONGLONG time;
        
        time = Int32x32To64 (timestamp, 10000000) + EPOCH_BIAS;
        
        fileTime->dwLowDateTime  = (DWORD) time;
        fileTime->dwHighDateTime = (DWORD) (time >> 32);
    }
}

void time_tToSystemTime (const time_t timestamp, SYSTEMTIME *systemTime)
{
    if (systemTime != NULL)
    {
        FILETIME fileTime;
        
        time_tToFileTime (timestamp, & fileTime);
        
        if (systemTime)
            FileTimeToSystemTime (& fileTime, systemTime);
    }
}

void time_tToLocalTime (const time_t timestamp, SYSTEMTIME *systemTime)
{
    if (systemTime != NULL)
    {
        FILETIME fileTime;
        
        time_tToLocalFileTime (timestamp, & fileTime);
        FileTimeToSystemTime (& fileTime, systemTime);
    }
}

time_t FileTimeTotime_t (FILETIME *fileTime)
{
    if (fileTime != NULL)
    {
        LARGE_INTEGER liTime;
        
        liTime.LowPart  = fileTime->dwLowDateTime;
        liTime.HighPart = fileTime->dwHighDateTime;

        _tzset ();
            
        return _timezone + (time_t) ((liTime.QuadPart - EPOCH_BIAS) / 10000000);
    }
    else
    {
        return 0;
    }
}

time_t LocalFileTimeTotime_t (FILETIME *fileTime)
{
    if (fileTime != NULL)
    {
        LARGE_INTEGER liTime;
        
        liTime.LowPart  = fileTime->dwLowDateTime;
        liTime.HighPart = fileTime->dwHighDateTime;

        return (time_t) ((liTime.QuadPart - EPOCH_BIAS) / 10000000);
    }
    else
    {
        return 0;
    }
}

time_t SystemTimeTotime_t (SYSTEMTIME *systemTime)
{
    if (systemTime != NULL)
    {
        FILETIME fileTime;
        
        if (SystemTimeToFileTime (systemTime, & fileTime))
            return FileTimeTotime_t (& fileTime);
        else
            return 0;
    }
    else
    {
        return 0;
    }
}

time_t LocalTimeTotime_t (SYSTEMTIME *systemTime)
{
    if (systemTime != NULL)
    {
        FILETIME fileTime;
        
        if (systemTime != NULL && SystemTimeToFileTime (systemTime, & fileTime))
            return FileTimeTotime_t (& fileTime);
        else
            return 0;
    }
    else
    {
        return 0;
    }
}

time_t LocalTimeTotime_t2 (SYSTEMTIME *systemTime)
{
    if (systemTime != NULL)
    {
        FILETIME fileTime;
        
        if (systemTime != NULL && SystemTimeToFileTime (systemTime, & fileTime))
            return LocalFileTimeTotime_t (& fileTime);
        else
            return 0;
    }
    else
    {
        return 0;
    }
}
