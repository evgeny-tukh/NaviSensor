#include "SerialPortPropsDlg.h"
#include "resource.h"

Tools::UIntArray baudRates { 1200, 2400, 4800, 9600, 14400, 28800, 38400, 115200 };
Tools::UIntArray byteSizes { 8, 7, 6 };

SerialPortPropsDlg::SerialPortPropsDlg (HINSTANCE instance, HWND parent, Sensors::SerialParams *params) :
    CDialogWrapper (instance, parent, IDD_SERIAL_PROPS),
    port (IDC_PORT),
    baud (IDC_BAUD),
    byteSize (IDC_BYTE_SIZE),
    parity (IDC_PARITY),
    stopBits (IDC_STOP_BITS)
{
    this->params = params;
}

LRESULT SerialPortPropsDlg::OnCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result = CDialogWrapper::OnCommand (wParam, lParam);

    return result;
}

BOOL SerialPortPropsDlg::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL           result = CDialogWrapper::OnInitDialog (wParam, lParam);
    Tools::Strings serialPorts;

    port.FindAndAttach (m_hwndHandle);
    baud.FindAndAttach (m_hwndHandle);
    byteSize.FindAndAttach(m_hwndHandle);
    parity.FindAndAttach (m_hwndHandle);
    stopBits.FindAndAttach (m_hwndHandle);

    Tools::getSerialPortsList(serialPorts);

    for (auto & portName : serialPorts)
    {
        const char *portNameStr = portName.c_str ();
        int         portNumber  = atoi (portNameStr + 3);
        int         item        = port.AddString (portNameStr, portNumber);

        if (params->port == portNumber)
            port.SetCurSel (item);
    }

    for (auto & baudRate : baudRates)
    {
        char buffer [50];
        int  item = baud.AddString (_itoa (baudRate, buffer, 10), baudRate);

        if (baudRate == params->baud)
            baud.SetCurSel (item);
    }

    for (auto & byteSizeVal : byteSizes)
    {
        char buffer [50];
        int  item = byteSize.AddString (_itoa (byteSizeVal, buffer, 10), byteSizeVal);

        if (byteSizeVal == params->byteSize)
            byteSize.SetCurSel (item);
    }

    for (auto & stopBitOption : Sensors::stopBitOptions)
    {
        int item = stopBits.AddString (stopBitOption.second, stopBitOption.first);

        if (stopBitOption.first == params->stopBits)
            stopBits.SetCurSel (item);
    }

    for (auto & parityOption : Sensors::parityOptions)
    {
        int item = parity.AddString (parityOption.second, parityOption.first);

        if (parityOption.first == params->parity)
            parity.SetCurSel(item);
    }

    return result;
}

void SerialPortPropsDlg::OnOK ()
{
    params->port     = port.GetSelectedData ();
    params->baud     = baud.GetSelectedData ();
    params->byteSize = byteSize.GetSelectedData ();
    params->parity   = (Sensors::Parity) parity.GetSelectedData ();
    params->stopBits = stopBits.GetSelectedData ();

    CDialogWrapper::OnOK ();
}

