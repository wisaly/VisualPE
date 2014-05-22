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
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnDropFiles( HDROP hDrop );
	void SetSelectFile(LPCTSTR pFileName);

private:
	CButtonUI *m_pSelectFile;
	CButtonUI *m_pTestbtn;
	CProgressUI *m_pScaleProgress;
	CContainerUI *m_pContainer;
	CLabelUI *m_pFilePath;
	ScalableLayout m_view;
};
