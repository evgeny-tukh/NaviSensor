#include <Windows.h>
#include "InputBox.h"
#include "resource.h"

CInputBox::CInputBox (HINSTANCE instance, HWND parent, const char *title, const char *prompt, char *buffer, const int size) : CDialogWrapper(instance, parent, IDD_INPUT_BOX)
{
    this->buffer = buffer;
    this->prompt = (char *) prompt;
    this->title  = (char *) title;
    this->size   = size;
}

LRESULT CInputBox::OnCommand (WPARAM wParam, LPARAM lParam)
{
    return CDialogWrapper::OnCommand (wParam, lParam);
}

BOOL CInputBox::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    SetItemText (IDC_TEXT, buffer);
    SetItemText (IDC_PROMPT, prompt);
    SetText (title);

    return CDialogWrapper::OnInitDialog (wParam, lParam);
}

void CInputBox::OnOK ()
{
    GetItemText (IDC_TEXT, buffer, size);

    CDialogWrapper::OnOK ();
}
