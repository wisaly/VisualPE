#pragma once

class ScalableNode
{
public:
	ScalableNode();
	typedef boost::shared_ptr<ScalableNode> Ptr;
	typedef vector<Ptr>::iterator Iter;

	int level_;
	bool isHor_;
	CDuiString name_;
	CDuiString text_;
	CDuiString tescription_;
	COLORREF bkColor_;
	vector<Ptr> children_;

	static Ptr FindChild(Ptr node,CDuiString name);
};

class ScalableLayout
{
public:
	ScalableLayout(CContainerUI *&pContainer);
	~ScalableLayout(void);

	void ShowLayout(CDuiString rootName = _T(""));
	void TestLayout();
private:
	CContainerUI *CreateLayout(ScalableNode::Ptr node,int level);
private:
	CContainerUI *&m_pContainer;
	ScalableNode::Ptr m_rootNode;
};
