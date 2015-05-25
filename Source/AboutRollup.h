/*/////////////////////////////////////////////////////////////////////
FILE: AboutRollup.h
DESCRIPTION: Adds an "About" rollout to a plugin
///////////////////////////////////////////////////////////////////////

To use:

Change ClassDesc to use "IDS_CATEGORY"

Add:
	IDS_VERSION			2001.mm.dd
	IDS_CATEGORY_BFD	BFDtools
	IDS_CATEGORY_FOO	fooTOOLS

Modifiers: ---------------------------------------
class member variable
	HWND hAboutRollup;

class:ctor:
	hAboutRollup = NULL;

class::BeginEditParams:
	hAboutRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ABOUT), aboutDlgProc, _T("About"));

class::EndEditParams:
	ip->DeleteRollupPage(hAboutRollup);

Textures: ---------------------------------------
class::CreateParamDlg:
	imp->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ABOUT), aboutDlgProc, _T("About"));

/////////////////////////////////////////////////////////////////////*/

#define FOOTOOL // Define to get a "fooTOOL", undefine to get a "BFDtool"

#ifdef FOOTOOL
#	define IDD_ABOUT		IDD_ABOUT_FOO
#	define IDS_CATEGORY		IDS_CATEGORY_FOO
#else
#	define IDD_ABOUT		IDD_ABOUT_BFD
#	define IDS_CATEGORY		IDS_CATEGORY_BFD
#endif

static INT_PTR CALLBACK aboutDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			TSTR ver = GetString(IDS_VERSION);
			HWND hTextWnd = GetDlgItem(hWnd, IDC_ABOUT_VERSION);
			SetWindowText(hTextWnd, ver);

			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_ABOUT_WEB:
				{
					const int Err = reinterpret_cast<int>( ShellExecute(hWnd, NULL, "http://www.footools.com/aboutplugin/bindpoints.html", NULL, NULL, NULL) );
					if (Err<32)
					{
						Interface* ip = GetCOREInterface();
						HWND maxhWnd = (ip) ? ip->GetMAXHWnd() : NULL;
						MessageBox(maxhWnd, "Internet access not available.", "Error", MB_OK|MB_ICONERROR|MB_APPLMODAL);
					}
					break;
				}
			}
			break;
		}
		default: return FALSE;
	}
	return TRUE;
}
