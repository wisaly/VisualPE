#include "StdAfx.h"
#include "ScalableLayout.h"

CScalableLayout::CScalableLayout(CContainerUI *&pContainer,CProgressUI *&pProgress,CHorizontalLayoutUI *&pStatusBar)
	:m_pContainer(pContainer),m_pProgress(pProgress),m_pStatusBar(pStatusBar)
{
}


CScalableLayout::~CScalableLayout(void)
{
}

CContainerUI * CScalableLayout::CreateLayout( CScalableNode::Ptr pNode,int nLevel )
{
	CContainerUI *pLayout = pNode->bHor ?
		static_cast<CContainerUI*>(new CHorizontalLayoutUI) : 
		static_cast<CContainerUI*>(new CVerticalLayoutUI);

	RECT rcInset={10,10,10,10};
	pLayout->SetInset(rcInset);
	pLayout->SetBkColor(pNode->crBk | 0xFF000000);

	for (CScalableNode::Iter i = pNode->ChildBegin();
		i != pNode->ChildEnd();
		i++)
	{
		bool bLeaf = true;
		for (CScalableNode::Iter j = (*i)->ChildBegin();
			j != (*i)->ChildEnd();
			j++)
		{
			if ((*j)->nLevel == nLevel)
			{
				bLeaf = false;
				break;
			}
		}
		if (bLeaf)
		{
			CContainerUI *pItemContainer = new CContainerUI;
			CButtonUI *pItem = new CButtonUI;
			pItem->SetName((*i)->sName);
			pItem->SetBkColor((*i)->crBk | 0xFF000000);
			pItem->SetText((*i)->sText);
			pItemContainer->Add(pItem);

			pLayout->Add(pItemContainer);
		}
		else
		{
			pLayout->Add(CreateLayout(*i,nLevel));
		}
	}

	return pLayout;
}

void CScalableLayout::ZoomIn( CDuiString sNodeName )
{
	if (!m_pRootNode)
	{
		return;
	}
	CScalableNode::Ptr pNode = m_pRootNode->FindChild(sNodeName);

	if (!pNode)
	{
		return;
	}

	m_pCurrentNode = pNode;
	m_pContainer->RemoveAll();
	m_pContainer->Add(CreateLayout(pNode,pNode->nLevel + 1));

	m_pStatusBar->SetVisible(pNode->nLevel > 0);
	m_pProgress->SetValue(m_nMaxLevel - pNode->nLevel + 1);
}

void CScalableLayout::ZoomOut()
{
	if (m_pCurrentNode && m_pCurrentNode->GetParent())
	{
		ZoomIn(m_pCurrentNode->GetParent()->sName);
	}
}

#define RANDCOLOR (RGB(rand()%255,rand()%255,rand()%255))
void CScalableLayout::TestLayout()
{
	CScalableNode::Ptr pRoot = 
		CScalableNode::New(0,true,RANDCOLOR)
			<< (CScalableNode::New(1,true,RANDCOLOR,_T("child1"),_T("child 1"))
			+ (CScalableNode::New(1,false,RANDCOLOR)
				<<(CScalableNode::New(1,true,RANDCOLOR,_T("pGchild1"),_T("grand child 1"))
				+ (CScalableNode::New(1,true,RANDCOLOR,_T("pGchild2"),_T("grand child 2"))
					<<(CScalableNode::New(2,true,RANDCOLOR,_T("ggchild1"),_T("grand grand child 1"))
					+ CScalableNode::New(2,true,RANDCOLOR,_T("ggchild2"),_T("grand grand child 2"))
					))
				))
			);


	SetContent(pRoot,2);
}

void CScalableLayout::SetContent( CScalableNode::Ptr pRoot,int nMaxLevel )
{
	m_pRootNode = pRoot;
	m_pCurrentNode = pRoot;
	m_nMaxLevel = nMaxLevel;

	m_pProgress->SetMinValue(0);
	m_pProgress->SetMaxValue(m_nMaxLevel + 1);

	ZoomIn(m_pRootNode->sName);
}