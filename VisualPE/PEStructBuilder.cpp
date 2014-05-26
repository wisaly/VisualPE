#include "StdAfx.h"
#include "PEStructBuilder.h"

CPEStructBuilder::CPEStructBuilder( CPEFile &pe )
	:m_pe(pe)
	//,m_rand(mt19937(),uniform_int<>(0,255))
{
	Build();
}

CPEStructBuilder::~CPEStructBuilder(void)
{
}

CScalableNode::Ptr CPEStructBuilder::Build()
{
	CScalableNode::Ptr pRoot = CScalableNode::New(
		0,true,RandColor(),_T(""),_T(""),_T("PE File"));

	CScalableNode::Ptr pDOSHeader = CScalableNode::New(
		1,false,RandColor(),_T(""),_T(""),_T("DOS Header"));
	CScalableNode::Ptr pNTHeader = CScalableNode::New(
		1,false,RandColor(),_T(""),_T(""),_T("NT Header"));

	CScalableNode::Ptr pSections = CScalableNode::New(
		1,false,RandColor(),_T(""),_T(""),_T("Sections"));
	
	pRoot << (pDOSHeader + pNTHeader + pSections);

	return pRoot;
}

COLORREF CPEStructBuilder::RandColor()
{
	return RGB(rand(),rand(),rand());
}