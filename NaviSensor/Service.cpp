#include <Shlwapi.h>
#include "Service.h"

Service *Service::instance = 0;

Service::Service (const char *name, const char *dsplyName, const unsigned long start)
{
    std::string eventName = std::string (name) + std::string ("_stopWorker");

    instance     = this;
    serviceName  = name;
    displayName  = dsplyName ? dsplyName : serviceName;
    startType    = start;
    statusHandle = 0;
    worker       = 0;
    stopWorker   = CreateEvent (0, 0, 0, eventName.c_str ());

    status.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState            = SERVICE_START_PENDING;
    status.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
    status.dwWin32ExitCode           = NO_ERROR;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint              = 0;
    status.dwWaitHint                = 0;
}

Service::~Service ()
{
    if (worker && worker->joinable ())
    {
        SetEvent (stopWorker);

        worker->join ();

        delete worker;
    }

    CloseHandle (stopWorker);
}

bool Service::install (const bool showStatus, const char *modulePath)
{
    char        path [MAX_PATH];
    SC_HANDLE   scManager = 0, service = 0;
    bool        result = false;
    const char *errorText;

    if (showStatus)
        printf ("Installing %s...", serviceName.c_str ());

    if (modulePath)
        strncpy (path, modulePath, sizeof (path));
    else
        GetModuleFileName (0, path, sizeof (path));

    if (PathFileExists (path))
    {
        scManager = OpenSCManager (0, 0, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);

        if (scManager)
        {
            service = CreateService (scManager, serviceName.c_str (), displayName.c_str (), SERVICE_QUERY_STATUS, SERVICE_WIN32_OWN_PROCESS, 
                                     startType, SERVICE_ERROR_NORMAL, path, 0, 0, 0, 0, 0);

            if (service)
            {
                result    = true;
                errorText = "";

                CloseServiceHandle (service);
            }
            else
            {
                errorText = "unable to create a service";
            }

            CloseServiceHandle (scManager);
        }
        else
        {
            errorText = "unable to connect SCM";
        }

        if (showStatus)
        {
            printf("%s.\n%s\n", errorText, result ? "Ok" : "Fail");

            if (!result)
                printf ("Error %d\n", GetLastError ());
        }
    }
    else if (showStatus)
    {
        printf ("'%s' not found.\n", path);
    }

    return result;
}

bool Service::uninstall (const bool showStatus)
{
    SC_HANDLE scManager   = 0, service = 0;
    bool      result      = false;
    SERVICE_STATUS status = {};

    if (showStatus)
        printf ("Uninstalling %s...\n", serviceName.c_str());

    scManager = OpenSCManager (0, 0, SC_MANAGER_CONNECT);

    if (scManager)
    {
        service = OpenService (scManager, serviceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);

        if (service)
        {
            if (QueryServiceStatus (service, & status) && status.dwCurrentState == SERVICE_RUNNING)
            {
                if (ControlService (service, SERVICE_CONTROL_STOP, & status))
                {
                    if (showStatus)
                        printf ("Stopping %s", serviceName.c_str ());

                    Sleep(1000);

                    while (QueryServiceStatus (service, & status) && status.dwCurrentState == SERVICE_STOP_PENDING)
                    {
                        if (showStatus)
                            printf (".");

                        Sleep (1000);
                    }

                    result = status.dwCurrentState == SERVICE_STOPPED;

                    if (showStatus)
                        printf ("%s.\n", result ?  "Ok" : "Fail");
                }
            }
            else
            {
                result = true;
            }

            if (result)
            {
                result = DeleteService (service) != 0;

                if (showStatus)
                    printf ("Deleting service...%s\n", result ? "Ok" : "Fail");
            }
            else if (showStatus)
            {
                printf ("Error %d stopping service.\n", GetLastError ());
            }

            CloseServiceHandle (service);
        }

        CloseServiceHandle (scManager);
    }

    return result;
}

void Service::start (unsigned int numOfArgs, const char **args)
{
    try
    {
        ResetEvent (stopWorker);

        setServiceStatus (SERVICE_START_PENDING);

        onStart (numOfArgs, args);

        worker  = new std::thread (workerProcInternal, this);

        setServiceStatus (SERVICE_RUNNING);
    }
    catch (unsigned long error)
    {
        writeErrorLogEntry ("Service Start", error);
        setServiceStatus (SERVICE_STOPPED, error);
    }
    catch (...)
    {
        writeEventLogEntry ("Service failed to start.", EVENTLOG_ERROR_TYPE);
        setServiceStatus (SERVICE_STOPPED);
    }

}

void Service::stop ()
{
    DWORD originalState = status.dwCurrentState;

    try
    {
        setServiceStatus (SERVICE_STOP_PENDING);

        onStop ();

        SetEvent (stopWorker);

        if (worker && worker->joinable())
        {
            worker->join ();

            delete worker;

            worker = 0;
        }

        setServiceStatus (SERVICE_STOPPED);
    }
    catch (unsigned long error)
    {
        writeErrorLogEntry ("Service Stop", error);
        setServiceStatus (originalState);
    }
    catch (...)
    {
        writeEventLogEntry ("Service failed to stop.", EVENTLOG_ERROR_TYPE);
        setServiceStatus (originalState);
    }
}

bool Service::startStop (const bool start)
{
    SC_HANDLE scManager   = 0, service = 0;
    bool      result      = false;
    SERVICE_STATUS status = {};

    scManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);

    if (scManager)
    {
        service = OpenService (scManager, serviceName.c_str(), SERVICE_START | SERVICE_STOP);

        if (service)
        {
            result = start ? (StartService (service, 0, 0) != 0) : (ControlService (service, SERVICE_CONTROL_STOP, & status) != 0);

            CloseServiceHandle (service);
        }

        CloseServiceHandle (scManager);
    }

    return result;
}

bool Service::installed (bool& running)
{
    SC_HANDLE scManager   = 0, service = 0;
    bool      result      = false;
    SERVICE_STATUS status = {};

    running   = false;
    scManager = OpenSCManager (0, 0, SC_MANAGER_CONNECT);

    if (scManager)
    {
        service = OpenService (scManager, serviceName.c_str(), SERVICE_QUERY_STATUS);

        if (service)
        {
            result = true;

            if (QueryServiceStatus (service, &status) && status.dwCurrentState == SERVICE_RUNNING)
                running = true;

            CloseServiceHandle (service);
        }

        CloseServiceHandle (scManager);
    }

    return result;
}

bool Service::run ()
{
    SERVICE_TABLE_ENTRY serviceTable [] = { { (char *) serviceName.c_str (), (SERVICE_MAIN_FUNCTION *) serviceMain }, { 0, 0 } };

    return StartServiceCtrlDispatcher (serviceTable) != 0;
}

void WINAPI Service::serviceMain (unsigned long numOfArgs, const char **args)
{
    if (instance)
    {
        instance->statusHandle = RegisterServiceCtrlHandler (instance->serviceName.c_str (), controlHandler);

        instance->start (numOfArgs, args);
    }
}

void WINAPI Service::controlHandler (unsigned long control)
{
    if (instance)
    {
        switch (control)
        {
            case SERVICE_CONTROL_STOP:
                instance->stop (); break;

            default:
                break;
        }
    }
}

void Service::setServiceStatus (unsigned long currentState, unsigned long exitCode, unsigned long waitHint)
{
    static unsigned long checkPoint = 1;

    status.dwCurrentState  = currentState;
    status.dwWin32ExitCode = exitCode;
    status.dwWaitHint      = waitHint;
    status.dwCheckPoint    = ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED)) ? 0 : checkPoint ++;

    ::SetServiceStatus (statusHandle, & status);
}

void Service::writeEventLogEntry (const char *message, unsigned long type)
{
    HANDLE      eventSource = RegisterEventSource (0, serviceName.c_str ());
    const char *strings [2] = { NULL, NULL };

    if (eventSource)
    {
        strings [0] = serviceName.c_str ();
        strings [1] = message;

        ReportEvent (eventSource, (unsigned short) type, 0, 0, 0, 2, 0, strings, 0);

        DeregisterEventSource (eventSource);
    }
}

void Service::writeErrorLogEntry (const char *function, unsigned long error)
{
    char message [260];
    
    snprintf (message, sizeof (message), "%s failed w/err 0x%08lx", function, error);

    writeEventLogEntry (message, EVENTLOG_ERROR_TYPE);
}

void Service::workerProcInternal (Service *self)
{
    if (self)
        self->workerProc ();
}

void Service::workerProc ()
{
    while (WaitForSingleObject (stopWorker, 0) != WAIT_OBJECT_0)
    {
        workerIteration ();

        std::this_thread::sleep_for (std::chrono::milliseconds (100));
    }
}
