#include "StdAfx.h"
#include "ScalableNode.h"


CScalableNode::CScalableNode(void)
{
}


CScalableNode::~CScalableNode(void)
{
}

CScalableNode::Ptr CScalableNode::New( int nLevel /*= 0*/, bool bHor /*= true*/, COLORREF crBk /*= 0*/, CDuiString sDescription /*= _T("")*/, DWORD dwSize /*= 0*/, CDuiString sName /*= _T("")*/, CDuiString sText /*= _T("")*/ )
{
	Ptr pNew = Ptr(new CScalableNode);
	pNew->m_pThis = pNew;
	pNew->Level = nLevel;
	pNew->IsHor = bHor;
	pNew->BkColor = crBk;
	pNew->Name = sName;
	pNew->Text = sText;
	pNew->Description = sDescription;
	//pNew->dwSize = dwSize;

	return pNew;
}
void CScalableNode::AppendChild( Ptr pNode )
{
	m_vChildren.push_back(pNode);
	pNode->m_pParent = m_pThis;
}

CScalableNode::Ptr CScalableNode::GetParent()
{
	return m_pParent;
}

CScalableNode::Iter CScalableNode::ChildBegin()
{
	return m_vChildren.begin();
}

CScalableNode::Iter CScalableNode::ChildEnd()
{
	return m_vChildren.end();
}

CScalableNode::Ptr CScalableNode::FindChild( CDuiString sName )
{
	if (this->Name == sName)
	{
		return m_pThis;
	}

	for (CScalableNode::Iter i = m_vChildren.begin();
		i != m_vChildren.end();
		i++)
	{
		Ptr pRet = (*i)->FindChild(sName);
		if (pRet)
		{
			return pRet;
		}
	}

	return Ptr();
}

CScalableNode::ListPtr operator+( CScalableNode::Ptr pLeft,CScalableNode::Ptr pRight )
{
	CScalableNode::ListPtr pList(new CScalableNode::List);
	pList->push_back(pLeft);
	pList->push_back(pRight);

	return pList;
}

CScalableNode::ListPtr operator+( CScalableNode::ListPtr pLeft,CScalableNode::Ptr pRight )
{
	pLeft->push_back(pRight);

	return pLeft;
}

CScalableNode::ListPtr operator+( CScalableNode::Ptr pLeft,CScalableNode::ListPtr pRight )
{
	pRight->push_back(pLeft);

	return pRight;
}

CScalableNode::ListPtr operator+( CScalableNode::ListPtr pLeft,CScalableNode::ListPtr pRight )
{
	for (CScalableNode::List::iterator i = pRight->begin();
		i != pRight->end();
		i++)
	{
		pLeft->push_back(*i);
	}

	return pLeft;
}

CScalableNode::Ptr operator<<( CScalableNode::Ptr pLeft,CScalableNode::Ptr pRight )
{
	pLeft->AppendChild(pRight);

	return pLeft;
}

CScalableNode::Ptr operator<<( CScalableNode::Ptr pLeft,CScalableNode::ListPtr pRight )
{
	for (CScalableNode::List::iterator i = pRight->begin();
		i != pRight->end();
		i++)
	{
		pLeft->AppendChild(*i);
	}

	return pLeft;
}