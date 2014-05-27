#pragma once

class CScalableNode : boost::noncopyable
{
	CScalableNode(void);
public:
	~CScalableNode(void);

	typedef boost::shared_ptr<CScalableNode> Ptr;
	typedef vector<Ptr> List;
	typedef boost::shared_ptr<List> ListPtr;
	typedef vector<Ptr>::iterator Iter;

	int Level;
	bool IsHor;
	CDuiString Name;
	CDuiString Text;
	CDuiString Description;
	COLORREF BkColor;

	static Ptr New(
		int nLevel = 0,
		bool bHor = true, 
		COLORREF crBk = 0, 
		CDuiString sDescription = _T(""), 
		DWORD dwSize = 0, 
		CDuiString sName = _T(""), 
		CDuiString sText = _T(""));
	void AppendChild(Ptr pNode);

	Ptr GetParent();
	Iter ChildBegin();
	Iter ChildEnd();

	Ptr FindChild(CDuiString sName);

	CDuiString SizeString(DWORD dwSize);

private:
	List m_vChildren;
	Ptr m_pThis;
	Ptr m_pParent;
};

CScalableNode::ListPtr operator+(CScalableNode::Ptr pLeft,CScalableNode::Ptr pRight);
CScalableNode::ListPtr operator+(CScalableNode::ListPtr pLeft,CScalableNode::Ptr pRight);
CScalableNode::ListPtr operator+(CScalableNode::Ptr pLeft,CScalableNode::ListPtr pRight);
CScalableNode::ListPtr operator+(CScalableNode::ListPtr pLeft,CScalableNode::ListPtr pRight);
CScalableNode::Ptr operator <<(CScalableNode::Ptr pLeft,CScalableNode::Ptr pRight);
CScalableNode::Ptr operator <<(CScalableNode::Ptr pLeft,CScalableNode::ListPtr pRight);
