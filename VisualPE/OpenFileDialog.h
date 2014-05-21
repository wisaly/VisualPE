#pragma once
#include <commdlg.h>
class COpenFileDialog
{
public:
	COpenFileDialog(HWND hwndOwner,LPCTSTR pFilter);
	~COpenFileDialog(void);

	BOOL Show();
	LPCTSTR GetFileName();

private:
	OPENFILENAME m_ofn;
	TCHAR m_szFileName[MAX_PATH];
};

