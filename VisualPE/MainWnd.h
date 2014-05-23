#pragma once
#include <shellapi.h>
#include "ScalableLayout.h"

class CMainWnd :
	public WindowImplBase
{
public:
	CMainWnd(void);
	~CMainWnd(void);

public:
    virtual LPCTSTR    GetWindowClassName() const;
    virtual CDuiString GetSkinFile();
    virtual CDuiString GetSkinFolder();
	virtual void InitWindow();
	virtual void OnClick(TNotifyUI& msg);
	void OnDropFiles( HDROP hDrop );
	virtual LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetSelectFile(LPCTSTR pFileName);

private:
	CButtonUI *m_pSelectFile;
	CButtonUI *m_pTestbtn;
	CButtonUI *m_pZoomout;
	CProgressUI *m_pScaleProgress;
	CContainerUI *m_pContainer;
	CLabelUI *m_pFilePath;
	CHorizontalLayoutUI *m_pStatusbar;
	CScalableLayout m_view;
};
