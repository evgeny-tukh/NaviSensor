#include "SerialReader.h"

#define ASCII_BEL       0x07 
#define ASCII_BS        0x08 
#define ASCII_LF        0x0A 
#define ASCII_CR        0x0D 
#define ASCII_XON       0x11 
#define ASCII_XOFF      0x13 

Readers::SerialReader::SerialReader (Sensors::SerialParams *config) : Reader(config)
{
    portHandle = INVALID_HANDLE_VALUE;
}

Readers::SerialReader::~SerialReader ()
{
}

size_t Readers::SerialReader::read ()
{
    COMSTAT       commState;
    unsigned long errorFlags, bytesRead, errorCode;
    bool          overflow;
    unsigned char buffer [0xFFFF];

    ClearCommError (portHandle, & errorFlags, & commState);

    overflow = (errorFlags & (CE_RXOVER | CE_OVERRUN)) != 0L;

    if (!ReadFile (portHandle, buffer, commState.cbInQue, & bytesRead, NULL))
    {
        errorCode = GetLastError ();
        bytesRead = 0;
    }

    if (bytesRead > 0)
        queue.pushBuffer (buffer, bytesRead);
        
    return bytesRead;
}

bool Readers::SerialReader::open ()
{
    Sensors::SerialParams *params = (Sensors::SerialParams *) config;
    char                   portName [50];

    sprintf (portName, "\\\\.\\COM%d", params->port);

    portHandle = CreateFile (portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (portHandle != INVALID_HANDLE_VALUE)
    {
        COMMTIMEOUTS timeouts;
        DCB          dcb;

        SetupComm (portHandle, 4096, 4096);
        PurgeComm (portHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

        memset (& dcb, 0, sizeof (dcb));

        GetCommState (portHandle, & dcb);

        dcb.BaudRate     = params->baud;
        dcb.ByteSize     = params->byteSize;
        dcb.StopBits     = params->stopBits;
        dcb.Parity       = params->parity;
        dcb.fBinary      = 1;
        dcb.fParity      = 1;
        dcb.fOutxDsrFlow = 0; 
        dcb.fDtrControl  = DTR_CONTROL_ENABLE;
        dcb.fOutxCtsFlow = 0; 
        dcb.fRtsControl  = RTS_CONTROL_ENABLE;
        dcb.fInX         =
        dcb.fOutX        = 1;
        dcb.XonChar      = ASCII_XON;
        dcb.XoffChar     = ASCII_XOFF;
        dcb.XonLim       = 100;
        dcb.XoffLim      = 100;

        SetCommState (portHandle, & dcb);

        GetCommTimeouts (portHandle, & timeouts);

        timeouts.ReadIntervalTimeout        = 1000;
        timeouts.ReadTotalTimeoutMultiplier = 1;
        timeouts.ReadTotalTimeoutConstant   = 3000;

        SetCommTimeouts (portHandle, & timeouts);

        EscapeCommFunction (portHandle, SETDTR);
    }

    opened = portHandle != INVALID_HANDLE_VALUE;

    return opened;
}

void Readers::SerialReader::close ()
{
    if (portHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle (portHandle);

        portHandle = INVALID_HANDLE_VALUE;
    }
}
