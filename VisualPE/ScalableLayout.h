#pragma once
#include "ScalableNode.h"

class CScalableLayout
{
public:
	CScalableLayout(
		CContainerUI *&pContainer,
		CProgressUI *&pProgress,
		CHorizontalLayoutUI *&pStatusBar);
	~CScalableLayout(void);

	void SetContent(CScalableNode::Ptr pRoot,int nMaxLevel);
	void ZoomIn(CDuiString sNodeName = _T(""));
	void ZoomOut();

	void TestLayout();
private:
	CContainerUI *CreateLayout(CScalableNode::Ptr pNode,int nLevel);
private:
	CContainerUI *&m_pContainer;
	CProgressUI *&m_pProgress;
	CHorizontalLayoutUI *&m_pStatusBar;
	CScalableNode::Ptr m_pRootNode;
	CScalableNode::Ptr m_pCurrentNode;
	int m_nMaxLevel;
};
