#include "StdAfx.h"
#include "MainWnd.h"
#include <time.h>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "OpenFileDialog.h"

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
	return _T("");
}

void CMainWnd::InitWindow()
{
	::SetWindowLong(*this, GWL_EXSTYLE, 
		::GetWindowLong(*this, GWL_EXSTYLE) | WS_EX_ACCEPTFILES);

	m_pSelectFile = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("selectfile")));
	m_pTestbtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("testbtn")));
	m_pScaleProgress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("scaleprogress")));
	m_pContainer = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container")));
	m_pFilePath = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("filepath")));
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
	else if (msg.pSender == m_pTestbtn)
	{
		CContainerUI * n = rand()%2 ? (CContainerUI *)(new CHorizontalLayoutUI) : (CContainerUI *)(new CVerticalLayoutUI);
		n->SetBorderSize(1);
		RECT r={10,10,10,10};
		n->SetInset(r);
		n->SetBkColor(RGB(rand()%255,rand()%255,rand()%255)|0xFF000000);
		n->SetMouseEnabled();
		

		CContainerUI *p = m_pContainer;
		int i;
		while ((i=rand() % (p->GetCount()+1)) != p->GetCount())
		{
			p = static_cast<CContainerUI*>(p->GetItemAt(i));
			
			if (rand()%2)
			{
				break;
			}
		}

		p->Add(n);
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

		boost::scoped_ptr<TCHAR> pFilePathName(
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