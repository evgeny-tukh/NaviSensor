#include "SensorInfoWnd.h"
#include "resource.h"

SensorInfoWnd::SensorInfoWnd (HINSTANCE instance, HWND parent, Sensors::SensorConfig *sensorCfg) :
        CDialogWrapper (instance, parent, IDD_SENSOR), sentences (IDC_SENTENCES), pause (IDC_PAUSE), terminal (IDC_TERMINAL),
        dataNode ((const unsigned int) Comm::Ports::CmdPort, onMessageInternal, (void *) this)
{
    this->sensorCfg = sensorCfg;
}

/*void SensorInfoWnd::logicProcInternal (Comm::CommCallback sendCb, Comm::CommCallback rcvCb, void *self)
{
    if (self)
        ((SensorInfoWnd *) self)->logicProc (sendCb, rcvCb);
}

void SensorInfoWnd::logicProc (Comm::CommCallback sendCb, Comm::CommCallback rcvCb)
{
    Tools::SimpleCall sendCmd =    [sendCb] (const byte cmd) -> int
                                   {
                                       return sendCb ((byte *) & cmd, (size_t) sizeof (cmd));
                                   },
                      waitForAck = [rcvCb] (const byte dummy = 0) -> int
                                   {
                                       byte data;

                                       while (rcvCb (& data, sizeof (data)) != sizeof (data) || data != Comm::Command::Ack && data != Comm::Command::Nak)
                                           Tools::sleepFor (5);

                                       return data == Comm::Command::Ack;
                                   };

    sendCmd (Comm::Command::StartInputFw);

    if (waitForAck (0))
    {
    }
}*/

void SensorInfoWnd::onMessage(Comm::MsgType msgType, const char *data, const int size)
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
    LRESULT result = CDialogWrapper::OnCommand(wParam, lParam);

    /*switch (LOWORD(wParam))
    {
    case IDC_CONNECTION:
    {
        if (HIWORD(wParam) == CBN_SELCHANGE)
            showParameterString();

        break;
    }

    case IDC_PARAMS_EDIT:
    {
        switch (connection.GetSelectedData())
        {
        case Sensors::File:
        {
            FilePropsDlg dialog(m_hInstance, m_hwndHandle, &sensorCfg->fileParam);

            if (dialog.Execute() == IDOK)
                paramString.SetText(sensorCfg->fileParam.getParameterString().c_str());

            break;
        }

        case Sensors::UDP:
        {
            UdpConnectionPropsDlg dialog(m_hInstance, m_hwndHandle, &sensorCfg->udpParam);

            if (dialog.Execute() == IDOK)
                paramString.SetText(sensorCfg->udpParam.getParameterString().c_str());

            break;
        }

        case Sensors::Serial:
        {
            SerialPortPropsDlg dialog(m_hInstance, m_hwndHandle, &sensorCfg->serialParam);

            if (dialog.Execute() == IDOK)
                paramString.SetText(sensorCfg->serialParam.getParameterString().c_str());

            break;
        }
        }

        break;
    }
    }*/

    return result;
}

BOOL SensorInfoWnd::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL result = CDialogWrapper::OnInitDialog (wParam, lParam);

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

    //if (!dataClient.connectTo ())
    //    MessageBox ("Unable to connect to sensor thread", "Error", MB_ICONSTOP);

    return result;
}

