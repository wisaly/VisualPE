// VisualPE.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "VisualPE.h"
#include "MainWnd.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	CPaintManagerUI::SetInstance(hInstance);

    CMainWnd mainWnd;
    mainWnd.Create(NULL, _T("VisualPE"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
    mainWnd.CenterWindow();
    mainWnd.ShowModal();
    return 0;
}