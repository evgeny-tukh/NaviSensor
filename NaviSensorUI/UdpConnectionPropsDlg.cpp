#include "UdpConnectionPropsDlg.h"
#include "resource.h"
#include "NicSelectDlg.h"

UdpConnectionPropsDlg::UdpConnectionPropsDlg (HINSTANCE instance, HWND parent, Sensors::UdpParams *params) :
    CDialogWrapper (instance, parent, IDD_UDP_PROPS),
    inPort (IDC_IN_PORT),
    outPort (IDC_OUT_PORT),
    inPortSpin (IDC_IN_PORT_SPIN),
    outPortSpin (IDC_OUT_PORT_SPIN),
    bind (IDC_BIND),
    dest (IDC_DEST)
{
    this->params = params;
}

LRESULT UdpConnectionPropsDlg::OnCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result = CDialogWrapper::OnCommand (wParam, lParam);

    if (LOWORD (wParam) == IDC_BROWSE)
    {
        in_addr      addr;
        NicSelectDlg dialog (m_hInstance, m_hwndHandle, & addr);

        if (dialog.Execute () == IDOK)
            bind.SetAddr (addr);
    }

    return result;
}

BOOL UdpConnectionPropsDlg::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL result = CDialogWrapper::OnInitDialog (wParam, lParam);

    inPort.FindAndAttach (m_hwndHandle);
    outPort.FindAndAttach (m_hwndHandle);
    inPortSpin.FindAndAttach (m_hwndHandle);
    outPortSpin.FindAndAttach (m_hwndHandle);
    bind.FindAndAttach (m_hwndHandle);
    dest.FindAndAttach (m_hwndHandle);

    inPort.SetInt (params->inPort);
    outPort.SetInt (params->outPort);

    inPortSpin.SendMessageA (UDM_SETRANGE32, 100, 100000);
    outPortSpin.SendMessageA (UDM_SETRANGE32, 100, 100000);

    bind.SetAddr (params->bind);
    dest.SetAddr (params->dest);

    return result;
}

void UdpConnectionPropsDlg::OnOK()
{
    params->inPort  = inPort.GetInt ();
    params->outPort = outPort.GetInt ();
    params->bind    = bind.GetAddr ();
    params->dest    = dest.GetAddr ();

    if (params->dest.S_un.S_addr == INADDR_ANY)
        params->dest.S_un.S_addr = INADDR_BROADCAST;

    CDialogWrapper::OnOK ();
}

