#include "SensorInfoWnd.h"
#include "resource.h"

SensorInfoWnd::SensorInfoWnd (HINSTANCE instance, HWND parent, Sensors::SensorConfig *sensorCfg) :
        CDialogWrapper (instance, parent, IDD_SENSOR), sentences (IDC_SENTENCES), pause (IDC_PAUSE), terminal (IDC_TERMINAL),
        dataNode ((const unsigned int) Comm::Ports::CmdPort, onMessageInternal, (void *) this)
{
    this->sensorCfg        = sensorCfg;
    this->threadEnabled    = true;
    this->listeningEnabled = false;
    this->rawDataReceiver  = new std::thread (receiverProcInternal, this);
}

SensorInfoWnd::~SensorInfoWnd()
{
    threadEnabled = false;

    if (rawDataReceiver && rawDataReceiver->joinable ())
        rawDataReceiver->join ();
}

void SensorInfoWnd::onMessage (Comm::MsgType msgType, const char *data, const int size)
{

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
        case IDC_PAUSE:
            listeningEnabled = IsDlgButtonChecked (IDC_PAUSE) == BST_UNCHECKED; break;
    }

    return result;
}

BOOL SensorInfoWnd::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL result = CDialogWrapper::OnInitDialog (wParam, lParam);
    char caption [256];

    GetText (caption, sizeof (caption));

    strcat (caption, ": ");
    strcat (caption, sensorCfg->getName ());

    SetText (caption);

    sentences.FindAndAttach (m_hwndHandle);
    pause.FindAndAttach (m_hwndHandle);
    terminal.FindAndAttach (m_hwndHandle);

    sentences.SetExtendedStyle (LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    sentences.AddColumn ("Type", 50);
    sentences.AddColumn ("Count", 50);
    sentences.AddColumn ("Last at", 170);

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

void SensorInfoWnd::receiverProc ()
{
    Comm::Socket receiver;
    char         buffer [2000];
    int          bytesReceived;
    in_addr      sender;

    receiver.create (Comm::Ports::RawDataFirstPort + sensorCfg->id ());

    requestRawData (true);

    while (threadEnabled)
    {
        bytesReceived = receiver.receiveFrom (buffer, sizeof (buffer), sender);

        if (bytesReceived > 0)
        {
            buffer [bytesReceived++] = '\n';
            buffer [bytesReceived++] = '\0';

            if (listeningEnabled)
                terminal.AddText (buffer, 10000);
        }

        Tools::sleepFor (10);
    }

    requestRawData (false);

    receiver.close ();
}

void SensorInfoWnd::receiverProcInternal (SensorInfoWnd *self)
{
    if (self)
        self->receiverProc ();
}
