#pragma once

#include <StdLib.h>
#include <time.h>
#include <Windows.h>

#if __cplusplus
    extern "C" {
#endif
    
void time_tToFileTime (const time_t timestamp, FILETIME *fileTime);
void time_tToLocalFileTime (const time_t timestamp, FILETIME *fileTime);
void time_tToSystemTime (const time_t timestamp, SYSTEMTIME *systemTime);
void time_tToLocalTime (const time_t timestamp, SYSTEMTIME *systemTime);
time_t FileTimeTotime_t (FILETIME *fileTime);
time_t LocalFileTimeTotime_t (FILETIME *fileTime);
time_t SystemTimeTotime_t (SYSTEMTIME *systemTime);
time_t LocalTimeTotime_t (SYSTEMTIME *systemTime);
time_t LocalTimeTotime_t2 (SYSTEMTIME *systemTime);

#if __cplusplus
    }
#endif
