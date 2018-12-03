#include <ctime>
#include "SensorInfoWnd.h"
#include "resource.h"

#define SENTENCE_UPDATE_TIMER   100

SensorInfoWnd::SensorInfoWnd (HINSTANCE instance, HWND parent, Sensors::SensorConfig *sensorCfg, Callback onDestroy) :
        CDialogWrapper (instance, parent, IDD_SENSOR), active(true), sentences (IDC_SENTENCES), pause (IDC_PAUSE), terminal (IDC_TERMINAL),
        dataNode ((const unsigned int) Comm::Ports::CmdPort, onMessageInternal, (void *) this)
{
    this->onDestroy             = onDestroy;
    this->sentenceUpdateTimer   = 0;
    this->sensorCfg             = sensorCfg;
    this->threadEnabled         = true;
    this->listeningEnabled      = false;
    this->rawDataReceiver       = new std::thread (rawDataReceiverProcInternal, this);
    this->sentenceStateReceiver = new std::thread (sentenceStateReceiverProcInternal, this);
}

SensorInfoWnd::~SensorInfoWnd ()
{
}

void SensorInfoWnd::stopThreads ()
{
    threadEnabled = false;

    if (rawDataReceiver && rawDataReceiver->joinable())
        rawDataReceiver->join ();

    if (sentenceStateReceiver && sentenceStateReceiver->joinable())
        sentenceStateReceiver->join ();

    delete rawDataReceiver, sentenceStateReceiver;
}

void SensorInfoWnd::onMessage (Comm::MsgType msgType, const char *data, const int size)
{

}

LRESULT SensorInfoWnd::OnDestroy ()
{
    if (onDestroy)
        onDestroy ();

    return CWindowWrapper::OnDestroy ();
}

void SensorInfoWnd::onMessageInternal (Comm::MsgType msgType, const char *data, const int size, void *param)
{
    if (param)
    {
        SensorInfoWnd *self = (SensorInfoWnd *) param;

        self->onMessage (msgType, data, size);
    }
}

LRESULT SensorInfoWnd::OnCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result = CDialogWrapper::OnCommand (wParam, lParam);

    switch (LOWORD(wParam))
    {
        case IDC_CLEAR:
            terminal.SetText (""); break;

        case IDC_REDETECT:
            redectectData (); break;

        case IDC_PAUSE:
            listeningEnabled = IsDlgButtonChecked (IDC_PAUSE) == BST_UNCHECKED; break;

        case IDCANCEL:
            if (sentenceUpdateTimer)
                KillTimer (m_hwndHandle, sentenceUpdateTimer);

            stopThreads ();

            active = false; 
            
            DestroyWindow (m_hwndHandle);

            if (onDestroy)
                onDestroy ();

            break;
    }

    return result;
}

BOOL SensorInfoWnd::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL result = CDialogWrapper::OnInitDialog (wParam, lParam);
    char caption [256];

    sentenceUpdateTimer = SetTimer (SENTENCE_UPDATE_TIMER, 1000);

    GetText (caption, sizeof (caption));

    strcat (caption, ": ");
    strcat (caption, sensorCfg->getName ());

    SetText (caption);

    sentences.FindAndAttach (m_hwndHandle);
    pause.FindAndAttach (m_hwndHandle);
    terminal.FindAndAttach (m_hwndHandle);

    sentences.SetExtendedStyle (LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    sentences.AddColumn ("Type", 40);
    sentences.AddColumn ("Count", 65);
    sentences.AddColumn ("Last at", 60);

    pause.Check ();

    CWindowWrapper::Show (SW_SHOW);
    CWindowWrapper::Update ();

    return result;
}

void SensorInfoWnd::requestRawData (const bool send)
{
    int sensorID = sensorCfg->id ();

    dataNode.sendCommand (Comm::CmdType::RawDataCtl, (byte) sensorID, (byte) (send ? 1 : 0),
                          (unsigned short) (Comm::Ports::RawDataFirstPort + sensorID), 8080, "127.0.0.1");
}

void SensorInfoWnd::requestSentenceState (const bool send)
{
    int sensorID = sensorCfg->id ();

    dataNode.sendCommand (Comm::CmdType::SentenceListCtl, (byte) sensorID, (byte) (send ? 1 : 0),
                         (unsigned short) (Comm::Ports::SentenceStateFirstPort + sensorID), 8080, "127.0.0.1");
}

void SensorInfoWnd::rawDataReceiverProc ()
{
    Comm::Socket receiver;
    char         buffer [2000];
    int          bytesReceived;
    in_addr      sender;

    receiver.create (Comm::Ports::RawDataFirstPort + sensorCfg->id ());

    requestRawData (true);

    while (threadEnabled)
    {
        unsigned int bytesAvailable = receiver.getAvalDataSize ();

        if (bytesAvailable > 0)
        {
            bytesReceived = receiver.receiveFrom (buffer, bytesAvailable > sizeof (buffer) ? sizeof (buffer) : bytesAvailable, sender);

            if (bytesReceived > 0)
            {
                buffer [bytesReceived++] = '\n';
                buffer [bytesReceived++] = '\0';

                if (listeningEnabled)
                    terminal.AddText (buffer, 10000);
            }
        }

        Tools::sleepFor (10);
    }

    requestRawData (false);

    receiver.close ();
}

void SensorInfoWnd::sentenceStateReceiverProc ()
{
    Comm::Socket               receiver;
    char                       buffer [2000];
    NMEA::SentenceStatus      *statuses = (NMEA::SentenceStatus *) buffer;
    int                        bytesReceived;
    in_addr                    sender;
    std::vector <unsigned int> sentenceItems;

    receiver.create (Comm::Ports::SentenceStateFirstPort + sensorCfg->id ());

    requestSentenceState (true);

    while (threadEnabled)
    {
        unsigned int bytesAvailable = receiver.getAvalDataSize();

        if (bytesAvailable > 0)
        {
            bytesReceived = receiver.receiveFrom (buffer, bytesAvailable > sizeof (buffer) ? sizeof (buffer) : bytesAvailable, sender);

            if (bytesReceived > 0)
            {
                //if (statusLock.try_lock ())
                {
                    statusLock.lock ();

                    curStatuses.clear ();

                    for (size_t i = 0, numOfSentences = bytesReceived / sizeof(NMEA::SentenceStatus); i < numOfSentences; ++ i)
                        curStatuses.push_back (statuses [i]);

                    statusLock.unlock ();
                }
            }
        }

        Tools::sleepFor (5);
    }

    requestSentenceState (false);

    receiver.close();
}

void SensorInfoWnd::rawDataReceiverProcInternal (SensorInfoWnd *self)
{
    if (self)
        self->rawDataReceiverProc ();
}

void SensorInfoWnd::sentenceStateReceiverProcInternal (SensorInfoWnd *self)
{
    if (self)
        self->sentenceStateReceiverProc ();
}

LRESULT SensorInfoWnd::OnSysCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = CDialogWrapper::OnSysCommand (wParam, lParam);

//    if (wParam == SC_CLOSE)
//        delete this;

    return lResult;
}

LRESULT SensorInfoWnd::OnTimer (UINT uiTimerID)
{
    if (uiTimerID == sentenceUpdateTimer)
    {
        size_t i, item;
        char   buffer[100];
        tm    *utc;

        //if (statusLock.try_lock ())
        {
            statusLock.lock ();

            for (i = 0; i < curStatuses.size (); ++ i)
            {
                auto & status = curStatuses [i];

                if (i >= (int) prevStatuses.size ())
                {
                    item = sentences.AddItem (status.type.name, status.type.type);

                    prevStatuses.push_back (status);
                }
                else if (status.type.type != prevStatuses [i].type.type)
                {
                    sentences.SetItemText (i, 0, status.type.name);
                    sentences.SetItemData (i, status.type.type);

                    item = i;

                    prevStatuses [i] = status;
                }
                else
                {
                    item = i;
                }

                utc = gmtime (& status.lastReceived);

                sprintf (buffer, "%02d:%02d:%02d", utc->tm_hour, utc->tm_min, utc->tm_sec);

                sentences.SetItemText (item, 1, std::to_string (status.count).c_str());
                sentences.SetItemText (item, 2, buffer);
            }

            statusLock.unlock ();
        }
    }

    return 0;
}

void SensorInfoWnd::redectectData()
{
    curStatuses.clear ();
    prevStatuses.clear ();

    sentences.DeleteAllItems ();
}

