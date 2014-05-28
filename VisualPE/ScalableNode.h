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
	DWORD BkColor;

	static Ptr New(
		int nLevel = 0,
		bool bHor = true, 
		DWORD crBk = 0, 
		CDuiString sDescription = _T(""),
		CDuiString sName = _T(""), 
		CDuiString sText = _T(""));
	
	void AppendChild(Ptr pNode);

	Ptr GetParent();
	bool HaveChildren();
	Iter ChildBegin();
	Iter ChildEnd();

	Ptr FindChild(CDuiString sName);

private:
	List m_vChildren;
	Ptr m_pThis;
	Ptr m_pParent;
};

// A + B : combine A B
// A << B : append B to A's children
// ~(A+B) : move A B to a horizontal layout
// !(A+B) : move A B to a vertical layout

CScalableNode::ListPtr operator+(CScalableNode::Ptr pLeft,CScalableNode::Ptr pRight);
CScalableNode::ListPtr operator+(CScalableNode::ListPtr pLeft,CScalableNode::Ptr pRight);
CScalableNode::ListPtr operator+(CScalableNode::Ptr pLeft,CScalableNode::ListPtr pRight);
CScalableNode::ListPtr operator+(CScalableNode::ListPtr pLeft,CScalableNode::ListPtr pRight);
CScalableNode::Ptr operator <<(CScalableNode::Ptr pLeft,CScalableNode::Ptr pRight);
CScalableNode::Ptr operator <<(CScalableNode::Ptr pLeft,CScalableNode::ListPtr pRight);
CScalableNode::Ptr operator ~(CScalableNode::ListPtr pItems);
CScalableNode::Ptr operator !(CScalableNode::ListPtr pItems);
CScalableNode::Ptr GRID(CScalableNode::ListPtr pItems,int nColumn);