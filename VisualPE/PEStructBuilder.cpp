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

void CPEStructBuilder::Build()
{
	CScalableNode::Ptr pRoot = CScalableNode::New(
		0,true,RandColor(),_T("PE File"));

	CScalableNode::Ptr pDOSHeader = CScalableNode::New(
		1,false,RandColor(),_T("DOS Header"));
	CScalableNode::Ptr pDOSStub = CScalableNode::New(
		1,false,RandColor(),_T("DOS Stub"));
	CScalableNode::Ptr pNTHeader = CScalableNode::New(
		1,false,RandColor(),_T("NT Header"));
	CScalableNode::Ptr pSectionTable = CScalableNode::New(
		1,false,RandColor(),_T("Section Table"));
	CScalableNode::Ptr pSections = CScalableNode::New(
		1,false,RandColor(),_T("Sections"));
	
	pRoot << (
		(CScalableNode::New(1,false,RandColor())
			<< (pDOSHeader + pDOSStub))
		+ pNTHeader + pSectionTable + pSections);

	pResult = pRoot;
	nMaxLevel = 1;
}

COLORREF CPEStructBuilder::RandColor()
{
	return RGB(rand(),rand(),rand());
}