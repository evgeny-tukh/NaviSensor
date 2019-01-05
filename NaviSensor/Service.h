#pragma once

#include <Windows.h>
#include <string>
#include <map>
#include <functional>
#include <thread>
#include <chrono>

class Service
{
    public:
        Service(const char *name, const char *displayName = 0, const unsigned long startType = SERVICE_AUTO_START);
        virtual ~Service();

        bool install (const bool showStatus = false, const char *path = 0);
        bool uninstall (const bool showStatus = false);
        void start (unsigned int numOgArgs = 0, const char **args = 0);
        void stop ();
        bool startStop (const bool start);
        bool run ();
        bool installed (bool& running);

        inline bool isRunning () { return WaitForSingleObject (stopWorker, 0) != WAIT_OBJECT_0; }

    protected:
        SERVICE_STATUS        status;
        SERVICE_STATUS_HANDLE statusHandle;
        std::string           serviceName, displayName;
        static Service       *instance;
        unsigned long         startType;
        std::thread          *worker;
        HANDLE                stopWorker;

        static void WINAPI serviceMain (unsigned long numOfArgs, const char **args);
        static void WINAPI controlHandler (unsigned long control);

        void setServiceStatus (unsigned long currentState, unsigned long exitCode = NO_ERROR, unsigned long waitHint = 0);

        void writeErrorLogEntry (const char *unction, unsigned long error);
        void writeEventLogEntry (const char *message, unsigned long type);

        virtual void onStart (unsigned int numOgArgs = 0, const char **args = 0) {}
        virtual void onStop () {}

        static void workerProcInternal (Service *);
        
        virtual void workerProc ();

        virtual void workerIteration () {}
};

