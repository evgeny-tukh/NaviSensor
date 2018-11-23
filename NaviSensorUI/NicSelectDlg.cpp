#include "NicSelectDlg.h"
#include "resource.h"
#include "tools.h"

NicSelectDlg::NicSelectDlg (HINSTANCE instance, HWND parent, in_addr *addr) :
    CDialogWrapper (instance, parent, IDD_NIC_SELECT),
    nicList (IDC_INTERFACES)
{
    this->addr = addr;
}

BOOL NicSelectDlg::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL             result = CDialogWrapper::OnInitDialog (wParam, lParam);
    Tools::AddrArray addrArray;
    in_addr          any, localhost;

    any.S_un.S_addr       = htonl (INADDR_ANY);
    localhost.S_un.S_addr = htonl (INADDR_LOOPBACK);

    addrArray.push_back (any);
    addrArray.push_back (localhost);

    Tools::getInterfaceList (addrArray);

    nicList.FindAndAttach (m_hwndHandle);

    for (auto & address : addrArray)
        nicList.AddString (inet_ntoa (address), address.S_un.S_addr);

    return result;
}

void NicSelectDlg::OnOK ()
{
    int selection = nicList.GetCurSel ();

    if (selection >= 0)
    {
        this->addr->S_un.S_addr = nicList.GetItemData (selection);

        CDialogWrapper::OnOK ();
    }
}

