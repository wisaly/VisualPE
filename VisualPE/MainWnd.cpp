#include "StdAfx.h"
#include "MainWnd.h"
#include <time.h>
#include <boost/scoped_array.hpp>
#include "OpenFileDialog.h"

CMainWnd::CMainWnd(void)
	:m_view(m_pContainer,m_pScaleProgress,m_pStatusbar)
{
}


CMainWnd::~CMainWnd(void)
{
}

LPCTSTR CMainWnd::GetWindowClassName() const
{
	return _T("CMainWnd");
}

DuiLib::CDuiString CMainWnd::GetSkinFile()
{
	return _T("MainFrame.xml");
}

DuiLib::CDuiString CMainWnd::GetSkinFolder()
{
	return _T("");
}

void CMainWnd::InitWindow()
{
	::SetWindowLong(*this, GWL_EXSTYLE, 
		::GetWindowLong(*this, GWL_EXSTYLE) | WS_EX_ACCEPTFILES);

	m_pSelectFile = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("selectfile")));
	m_pZoomout = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("zoomout")));
	m_pTestbtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("testbtn")));
	m_pScaleProgress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("scaleprogress")));
	m_pContainer = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container")));
	m_pFilePath = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("filepath")));
	m_pStatusbar = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("statusbar")));
}

void CMainWnd::OnClick( TNotifyUI& msg )
{
	__super::OnClick(msg);

	if (msg.pSender == m_pSelectFile)
	{
		COpenFileDialog dlg(*this,
			_T("Execution file name(*.exe)\0*.exe\0\0"));
		if (dlg.Show())
		{
			SetSelectFile(dlg.GetFileName());
		}
	}
	else if (msg.pSender == m_pZoomout)
	{
		m_view.ZoomOut();
	}
	else if (msg.pSender == m_pTestbtn)
	{
		m_view.TestLayout();
	}
	else
	{
		m_view.ZoomIn(msg.pSender->GetName());
	}
}

LRESULT CMainWnd::OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	WORD fwKeys = LOWORD(wParam);    // key flags
	if (fwKeys != 0)
	{
		return 0;
	}

	short zDelta = (short) HIWORD(wParam);    // wheel rotation

	POINT pt;
	pt.x = (short) LOWORD(lParam);    // horizontal position of pointer
	pt.y = (short) HIWORD(lParam);    // vertical position of pointer

	::ScreenToClient(*this,&pt);

	if (zDelta > 0)
	{
		// zoom in
		CControlUI *pHit = m_PaintManager.FindControl(pt);

		CButtonUI *pHitButton = dynamic_cast<CButtonUI*>(pHit);
		if (pHitButton == 0)
		{
			return 0;
		}

		m_view.ZoomIn(pHitButton->GetName());
	}
	else
	{
		RECT rc = m_pContainer->GetPos();
		if (!::PtInRect(&rc,pt))
		{
			return 0;
		}

		m_view.ZoomOut();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT CMainWnd::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_DROPFILES)
	{
		OnDropFiles((HDROP)wParam);
		return 0;
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CMainWnd::OnDropFiles( HDROP hDrop )
{
	WORD wNumFilesDropped = DragQueryFile(hDrop, -1, NULL, 0);  
	WORD wPathnameSize = 0;

	// there may be many, but we'll only use the first  
	if (wNumFilesDropped > 0)  
	{  
		wPathnameSize = DragQueryFile(hDrop, 0, NULL, 0);  
		wPathnameSize++;

		boost::scoped_array<TCHAR> pFilePathName(
			new TCHAR[wPathnameSize]);
		if (NULL == pFilePathName)  
		{  
			ASSERT(0);
			DragFinish(hDrop);  
			return;  
		}  

		//::ZeroMemory(pFilePathName, wPathnameSize);  
		DragQueryFile(hDrop, 0, pFilePathName.get(), wPathnameSize);
		SetSelectFile(pFilePathName.get());
	}  

	DragFinish(hDrop);
}

void CMainWnd::SetSelectFile( LPCTSTR pFileName )
{
	m_pContainer->RemoveAll();

	m_pFilePath->SetText(pFileName);
}