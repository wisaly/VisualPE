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
		1,false,RandColor(),CombineSize(_T("DOS Header"),m_pe.DosHeader.GetSize()),_T("DOS Header"));
	CScalableNode::Ptr pDOSStub = CScalableNode::New(
		1,false,RandColor(),CombineSize(_T("DOS Stub"),m_pe.DosStub.GetSize()),_T("DOS Stub"));
	CScalableNode::Ptr pNTHeader = CScalableNode::New(
		1,false,RandColor(),CombineSize(_T("NT Header"),m_pe.NtHeader.GetSize()),_T("NT Header"));
	CScalableNode::Ptr pCoffHeader = CScalableNode::New(
		1,false,RandColor(),_T("Coff Header"),_T("Coff Header"));
	CScalableNode::Ptr pOptionalHeader = CScalableNode::New(
		1,false,RandColor(),_T("Optional Header"),_T("Optional Header"));
	CScalableNode::Ptr pDataDirectory = CScalableNode::New(
		1,false,RandColor(),_T("Data Directory"),_T("Data Directory"));
	CScalableNode::Ptr pSectionTable = CScalableNode::New(
		1,false,RandColor(),_T("Section Table"),_T("Section Table"));
	CScalableNode::Ptr pSections = CScalableNode::New(
		1,false,RandColor(),_T("Sections"),_T("Sections"));
	
	pRoot << (
		(CScalableNode::New(1,false,RandColor())<< (
			pDOSHeader 
			+ pDOSStub))
		+ (pNTHeader << (
			pCoffHeader 
			+ pOptionalHeader 
			+ pDataDirectory))
		+ pSectionTable 
		+ pSections)
		<< GRID(
		CScalableNode::New(1,true,RandColor()) + 
		CScalableNode::New(1,true,RandColor()) + 
		CScalableNode::New(1,true,RandColor()) + 
		CScalableNode::New(1,true,RandColor()) + 
		CScalableNode::New(1,true,RandColor()) + 
		CScalableNode::New(1,true,RandColor()) + 
		CScalableNode::New(1,true,RandColor()),2);

	pResult = pRoot;
	nMaxLevel = 1;
}

DWORD CPEStructBuilder::RandColor()
{
	return 0xFF000000 | RGB(rand(),rand(),rand());
}
#define MEGA (2 << 19)
#define KILO (2 << 9)

CDuiString CPEStructBuilder::Size2String(DWORD dwSize)
{
	CDuiString sResult;
	if (dwSize > MEGA)
	{
		sResult.Format(_T("%d MB"),
			dwSize / MEGA);
	}
	else if (dwSize > KILO)
	{
		sResult.Format(_T("%d KB"),
			dwSize / KILO);
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