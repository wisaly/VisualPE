#include "StdAfx.h"
#include "ScalableLayout.h"

ScalableNode::ScalableNode()
{

}

ScalableNode::Ptr ScalableNode::FindChild( Ptr node,CDuiString name )
{
	if (node->name_ == name)
	{
		return node;
	}

	for (ScalableNode::Iter i = node->children_.begin();
		i != node->children_.end();
		i++)
	{
		Ptr ret = FindChild(*i,name);
		if (ret)
		{
			return ret;
		}
	}

	return Ptr();
}
ScalableLayout::ScalableLayout(CContainerUI *&pContainer)
	:m_pContainer(pContainer)
{
}


ScalableLayout::~ScalableLayout(void)
{
}

CContainerUI * ScalableLayout::CreateLayout( ScalableNode::Ptr node,int level )
{
	CContainerUI *layout = node->isHor_ ?
		static_cast<CContainerUI*>(new CHorizontalLayoutUI) : 
		static_cast<CContainerUI*>(new CVerticalLayoutUI);

	layout->SetBkColor(node->bkColor_);
	RECT r={10,10,10,10};
	layout->SetInset(r);

	for (ScalableNode::Iter i = node->children_.begin();
		i != node->children_.end();
		i++)
	{
		bool isLeaf = true;
		for (ScalableNode::Iter j = (*i)->children_.begin();
			j != (*i)->children_.end();
			j++)
		{
			if ((*j)->level_ == level)
			{
				isLeaf = false;
				break;
			}
		}
		if (isLeaf)
		{
			CContainerUI *itemContainer = new CContainerUI;
			CButtonUI *item = new CButtonUI;
			item->SetName((*i)->name_);
			item->SetBkColor((*i)->bkColor_);
			item->SetText((*i)->text_);
			itemContainer->Add(item);

			layout->Add(itemContainer);
		}
		else
		{
			layout->Add(CreateLayout(*i,level));
		}
	}

	return layout;
}

void ScalableLayout::ShowLayout( CDuiString rootName )
{
	m_pContainer->RemoveAll();

	ScalableNode::Ptr node = ScalableNode::FindChild(m_rootNode,rootName);
	m_pContainer->Add(CreateLayout(node,node->level_ + 1));
}

#define RANDCOLOR (RGB(rand()%255,rand()%255,rand()%255)|0xFF000000)
void ScalableLayout::TestLayout()
{
	m_rootNode = ScalableNode::Ptr(new ScalableNode);
	m_rootNode->level_ = 0;
	m_rootNode->isHor_ = true;
	m_rootNode->bkColor_ = RANDCOLOR;

	ScalableNode::Ptr pChild1 = ScalableNode::Ptr(new ScalableNode);
	pChild1->level_ = 1;
	pChild1->name_ = _T("child1");
	pChild1->text_ = _T("child 1");
	pChild1->bkColor_ = RANDCOLOR;
	m_rootNode->children_.push_back(pChild1);

	ScalableNode::Ptr pChild2 = ScalableNode::Ptr(new ScalableNode);
	pChild2->level_ = 1;
	pChild2->isHor_ = false;
	pChild2->bkColor_ = RANDCOLOR;
	m_rootNode->children_.push_back(pChild2);

	ScalableNode::Ptr pGchild1 = ScalableNode::Ptr(new ScalableNode);
	pGchild1->level_ = 1;
	pGchild1->name_ = _T("pGchild1");
	pGchild1->text_ = _T("grand child 1");
	pGchild1->bkColor_ = RANDCOLOR;
	pChild2->children_.push_back(pGchild1);


	ScalableNode::Ptr pGchild2 = ScalableNode::Ptr(new ScalableNode);
	pGchild2->level_ = 1;
	pGchild2->name_ = _T("pGchild2");
	pGchild2->text_ = _T("grand child 2");
	pGchild2->bkColor_ = RANDCOLOR;
	pChild2->children_.push_back(pGchild2);
}