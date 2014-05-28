#pragma once
#include "ScalableNode.h"

class CScalableLayout : 
	public CContainerUI,public INotifyUI,public IMessageFilterUI
{
public:
	CScalableLayout(HWND hParentWnd);
	~CScalableLayout(void);

	
	void SetContent(CScalableNode::Ptr pRoot,int nMaxLevel);
	void ZoomIn(CDuiString sNodeName = _T(""));
	void ZoomIn(CScalableNode::Ptr pNode);
	void ZoomOut();

	void TestLayout();

protected:
	void DoInit();
	void Notify(TNotifyUI& msg);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

private:
	CContainerUI *CreateLayout(CScalableNode::Ptr pNode,int nLevel);
private:
	CContainerUI *m_pContainer;
	CProgressUI *m_pProgress;
	CHorizontalLayoutUI *m_pStatusBar;
	CButtonUI *m_pZoomout;
	CScalableNode::Ptr m_pRootNode;
	CScalableNode::Ptr m_pCurrentNode;
	int m_nMaxLevel;
	HWND m_hParentWnd;
};
