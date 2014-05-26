#include "StdAfx.h"
#include "MainWnd.h"
#include <time.h>
#include <boost/scoped_array.hpp>
#include "OpenFileDialog.h"
#include "PEStructBuilder.h"

CMainWnd::CMainWnd(void)
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
	return _T("res");
}

void CMainWnd::InitWindow()
{
	::SetWindowLong(*this, GWL_EXSTYLE, 
		::GetWindowLong(*this, GWL_EXSTYLE) | WS_EX_ACCEPTFILES);

	m_pSelectFile = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("selectfile")));
	m_pTestbtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("testbtn")));
	m_pFilePath = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("filepath")));
	
	m_pView = static_cast<CScalableLayout*>(m_PaintManager.FindControl(_T("mainview")));
}

CControlUI * CMainWnd::CreateControl( LPCTSTR pstrClass )
{
	if (_tcsicmp(pstrClass, _T("ScalableLayout")) == 0)
	{
		return new CScalableLayout(*this);
	}

	return NULL;
}

void CMainWnd::OnClick( TNotifyUI& msg )
{
	__super::OnClick(msg);

	if (msg.pSender == m_pSelectFile)
	{
		COpenFileDialog dlg(*this,
			_T("PE file(*.exe;*.dll)\0*.exe;*.dll\0\0"));
		if (dlg.Show())
		{
			SetSelectFile(dlg.GetFileName());
		}
	}
	else if (msg.pSender == m_pTestbtn)
	{
		m_pView->TestLayout();
	}
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

void CMainWnd::SetSelectFile( CDuiString sFileName )
{
	if (sFileName.Right(4).CompareNoCase(_T(".exe")) != 0 &&
		sFileName.Right(4).CompareNoCase(_T(".dll")) != 0)
	{
		ErrorMessage(_T("Only exe or dll files are accepted"));
		return;
	}

	CPEFile pe;
	if (!pe.LoadFile(sFileName))
	{
		ErrorMessage(_T("Failed to load file"));
		return;
	}
	
	m_pFilePath->SetText(sFileName);

	CPEStructBuilder builder(pe);
	m_pView->SetContent(builder.pResult,builder.nMaxLevel);
}

void CMainWnd::ErrorMessage( LPCTSTR pText )
{
	::MessageBox(*this,
		pText,
		_T("Error"),
		MB_OK|MB_ICONERROR);
}