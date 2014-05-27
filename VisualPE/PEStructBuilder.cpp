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
		0,true,RandColor(),CombineSize(_T("PE File"),m_pe.FileBuf.GetSize()));

	CScalableNode::Ptr pDOSHeader = CScalableNode::New(
		1,false,RandColor(),CombineSize(_T("DOS Header"),m_pe.DosHeader.GetSize()));
	CScalableNode::Ptr pDOSStub = CScalableNode::New(
		1,false,RandColor(),CombineSize(_T("DOS Stub"),m_pe.DosStub.GetSize()));
	CScalableNode::Ptr pNTHeader = CScalableNode::New(
		1,false,RandColor(),CombineSize(_T("NT Header"),m_pe.NtHeader.GetSize()));
	CScalableNode::Ptr pCoffHeader = CScalableNode::New(
		1,false,RandColor(),_T("Coff Header"));
	CScalableNode::Ptr pOptionalHeader = CScalableNode::New(
		1,false,RandColor(),_T("Optional Header"));
	CScalableNode::Ptr pDataDirectory = CScalableNode::New(
		1,false,RandColor(),_T("Data Directory"));
	CScalableNode::Ptr pSectionTable = CScalableNode::New(
		1,false,RandColor(),_T("Section Table"));
	CScalableNode::Ptr pSections = CScalableNode::New(
		1,false,RandColor(),_T("Sections"));
	
	pRoot << (
		(CScalableNode::New(1,false,RandColor())
			<< (pDOSHeader + pDOSStub))
		+ (pNTHeader << 
			(pCoffHeader + pOptionalHeader + pDataDirectory))
		+ pSectionTable + pSections);

	pResult = pRoot;
	nMaxLevel = 1;
}

COLORREF CPEStructBuilder::RandColor()
{
	return RGB(rand(),rand(),rand());
}

CDuiString CPEStructBuilder::Size2String(DWORD dwSize)
{
	CDuiString sResult;
	if (dwSize > (2 << 20))
	{
		sResult.Format(_T("%d MB"),dwSize);
	}
	else if (dwSize > (2 << 10))
	{
		sResult.Format(_T("%d KB"),dwSize);
	}
	else
	{
		sResult.Format(_T("%d bytes"),dwSize);
	}

	return sResult;
}

CDuiString CPEStructBuilder::CombineSize( CDuiString sDescription,DWORD dwSize )
{
	CDuiString sResult;
	sResult.Format(_T("%s (%s)"),
		(LPCTSTR)sDescription,
		(LPCTSTR)Size2String(dwSize));

	return sResult;
}