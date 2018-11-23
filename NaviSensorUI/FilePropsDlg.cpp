#include "FilePropsDlg.h"
#include "resource.h"

FilePropsDlg::FilePropsDlg (HINSTANCE instance, HWND parent, Sensors::FileParams *params) :
    CDialogWrapper (instance, parent, IDD_FILE_PROPS),
    path (IDC_PATH),
    pause (IDC_PAUSE),
    pauseSpin (IDC_PAUSE_SPIN)
{
    this->params = params;
}

LRESULT FilePropsDlg::OnCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result = CDialogWrapper::OnCommand (wParam, lParam);

    switch (LOWORD (wParam))
    {
        case IDC_BROWSE:
        {
            char buffer [MAX_PATH];

            path.GetText (buffer, sizeof (buffer));

            std::string newPath = Tools::browseForFile (m_hwndHandle, "Log files (*.log)|*.log|All files|*.*|", "Select file", buffer);

            if (!newPath.empty ())
                path.SetText (newPath.c_str ());

            break;
        }
    }

    return result;
}

BOOL FilePropsDlg::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL result = CDialogWrapper::OnInitDialog (wParam, lParam);

    path.FindAndAttach (m_hwndHandle);
    pause.FindAndAttach (m_hwndHandle);
    pauseSpin.FindAndAttach (m_hwndHandle);

    path.SetText (params->filePath.c_str ());
    pause.SetInt (params->pauseBetwenLines);

    pauseSpin.SendMessage (UDM_SETRANGE32, 0, 10000);

    return result;
}

void FilePropsDlg::OnOK()
{
    char buffer [MAX_PATH];

    path.GetText (buffer, sizeof (buffer));

    params->filePath         = buffer;
    params->pauseBetwenLines = pause.GetInt ();

    CDialogWrapper::OnOK ();
}

