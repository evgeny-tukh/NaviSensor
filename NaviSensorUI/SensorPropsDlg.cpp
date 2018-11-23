#include "SensorPropsDlg.h"
#include "SerialPortPropsDlg.h"
#include "UdpConnectionPropsDlg.h"
#include "FilePropsDlg.h"
#include "resource.h"

SensorPropsDlg::SensorPropsDlg (HINSTANCE instance, HWND parent, Sensors::SensorConfig *sensorCfg) :
    CDialogWrapper (instance, parent, IDD_SENSOR_PROPS),
    type (IDC_TYPE),
    connection (IDC_CONNECTION),
    name (IDC_NAME),
    paramString (IDC_PARAMS)
{
    this->sensorCfg = sensorCfg;
}

LRESULT SensorPropsDlg::OnCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result = CDialogWrapper::OnCommand (wParam, lParam);

    switch (LOWORD (wParam))
    {
        case IDC_CONNECTION:
        {
            if (HIWORD (wParam) == CBN_SELCHANGE)
                showParameterString ();
                
            break;
        }

        case IDC_PARAMS_EDIT:
        {
            switch (connection.GetSelectedData ())
            {
                case Sensors::File:
                {
                    FilePropsDlg dialog (m_hInstance, m_hwndHandle, & sensorCfg->fileParam);

                    if (dialog.Execute() == IDOK)
                        paramString.SetText (sensorCfg->fileParam.getParameterString ().c_str ());

                    break;
                }

                case Sensors::UDP:
                {
                    UdpConnectionPropsDlg dialog (m_hInstance, m_hwndHandle, & sensorCfg->udpParam);

                    if (dialog.Execute() == IDOK)
                        paramString.SetText (sensorCfg->udpParam.getParameterString ().c_str ());

                    break;
                }

                case Sensors::Serial:
                {
                    SerialPortPropsDlg dialog (m_hInstance, m_hwndHandle, & sensorCfg->serialParam);

                    if (dialog.Execute() == IDOK)
                        paramString.SetText (sensorCfg->serialParam.getParameterString ().c_str ());

                    break;
                }
            }

            break;
        }
    }

    return result;
}

BOOL SensorPropsDlg::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL result = CDialogWrapper::OnInitDialog (wParam, lParam);

    type.FindAndAttach (m_hwndHandle);
    connection.FindAndAttach (m_hwndHandle);
    name.FindAndAttach (m_hwndHandle);
    paramString.FindAndAttach (m_hwndHandle);

    type.AddString ("NMEA0183", Sensors::NMEA);
    type.AddString ("AIS", Sensors::AIS);

    connection.AddString ("Serial", Sensors::Serial);
    connection.AddString ("UDP", Sensors::UDP);
    connection.AddString ("File", Sensors::File);

    name.SetText (sensorCfg->getName ());

    for (int i = 0; i < type.GetCount(); ++i)
    {
        if (type.GetItemData (i) == sensorCfg->type)
        {
            type.SetCurSel (i); break;
        }
    }

    for (int i = 0; i < connection.GetCount(); ++i)
    {
        if (connection.GetItemData (i) == sensorCfg->connection)
        {
            connection.SetCurSel (i); break;
        }
    }

    showParameterString ();

    return result;
}

void SensorPropsDlg::showParameterString ()
{
    switch (connection.GetSelectedData ())
    {
        case Sensors::File:
            paramString.SetText (sensorCfg->fileParam.getParameterString ().c_str ()); break;

        case Sensors::UDP:
                paramString.SetText (sensorCfg->udpParam.getParameterString ().c_str ()); break;

        case Sensors::Serial:
            paramString.SetText (sensorCfg->serialParam.getParameterString ().c_str ()); break;
    }
}

void SensorPropsDlg::OnOK ()
{
    char nameBuf [100];

    sensorCfg->connection = (Sensors::Connection) connection.GetSelectedData ();
    sensorCfg->type       = (Sensors::Type) type.GetSelectedData ();

    name.GetText (nameBuf, sizeof (nameBuf));

    sensorCfg->name = nameBuf;

    CDialogWrapper::OnOK ();
}

