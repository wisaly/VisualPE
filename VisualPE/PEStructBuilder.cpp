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

#define NODE(...) CScalableNode::New(__VA_ARGS__)

void CPEStructBuilder::Build()
{
	UniqueIdInit();
	CScalableNode::Ptr pRoot = NODE(
		0,true,RandColor(),CombineSize(_T("PE File"),m_pe.FileBuf.GetSize()),UniqueId());

	CScalableNode::Ptr pDOSHeader = NODE(
		1,false,RandColor(),CombineSize(_T("DOS Header"),m_pe.DosHeader.GetSize()),UniqueId());

	pDOSHeader << ((
		NODE(2,false,RandColor(),_T("Magic number"),_T(""),_T("'MZ'")) + 
		NODE(2,false,RandColor(),_T("Bytes on last page of file"),_T(""),Num2String(m_pe.DosHeader->e_cblp)) + 
		NODE(2,false,RandColor(),_T("Pages in file"),_T(""),Num2String(m_pe.DosHeader->e_cp)) + 
		NODE(2,false,RandColor(),_T("Relocations"),_T(""),Num2String(m_pe.DosHeader->e_crlc)) + 
		NODE(2,false,RandColor(),_T("Size of header in paragraphs"),_T(""),Num2String(m_pe.DosHeader->e_cparhdr)) + 
		NODE(2,false,RandColor(),_T("Minimum extra paragraphs needed"),_T(""),Num2String(m_pe.DosHeader->e_minalloc)) + 
		NODE(2,false,RandColor(),_T("Maximum extra paragraphs needed"),_T(""),Num2String(m_pe.DosHeader->e_maxalloc)) + 
		NODE(2,false,RandColor(),_T("Initial (relative) SS value"),_T(""),Num2String(m_pe.DosHeader->e_ss)) + 
		NODE(2,false,RandColor(),_T("Initial SP value"),_T(""),Num2String(m_pe.DosHeader->e_sp)) + 
		NODE(2,false,RandColor(),_T("Checksum"),_T(""),Num2String(m_pe.DosHeader->e_csum)) + 
		NODE(2,false,RandColor(),_T("Initial IP value"),_T(""),Num2String(m_pe.DosHeader->e_ip)) + 
		NODE(2,false,RandColor(),_T("Initial (relative) CS value"),_T(""),Num2String(m_pe.DosHeader->e_cs)) + 
		NODE(2,false,RandColor(),_T("File address of relocation table"),_T(""),Num2String(m_pe.DosHeader->e_lfarlc)) + 
		NODE(2,false,RandColor(),_T("Overlay number"),_T(""),Num2String(m_pe.DosHeader->e_ovno)) + 
		NODE(2,false,RandColor(),_T("OEM identifier"),_T(""),Num2String(m_pe.DosHeader->e_oemid)) + 
		NODE(2,false,RandColor(),_T("OEM information"),_T(""),Num2String(m_pe.DosHeader->e_oeminfo)) + 
		NODE(2,false,RandColor(),_T("File address of new exe header"),_T(""),Num2String(m_pe.DosHeader->e_lfanew))) / 4
		);

	CScalableNode::Ptr pDOSStub = CScalableNode::New(
		1,false,RandColor(),CombineSize(_T("DOS Stub"),m_pe.DosStub.GetSize()),UniqueId());

	{
		int nColumns = (int)sqrt((float)m_pe.DosStub.GetSize());
		CScalableNode::ListPtr pItems(new CScalableNode::List);
		DWORD dwColor = RandColor();
		for (int i = 0;i < m_pe.DosStub.GetSize();i++)
		{
			pItems = pItems + 
				NODE(2,false,dwColor,Num2String(m_pe.DosStub[i]));
		}

		pDOSStub << (pItems % nColumns);
	}

	CScalableNode::Ptr pNTHeader = NODE(
		1,false,RandColor(),CombineSize(_T("NT Header"),m_pe.NtHeader.GetSize()),UniqueId());
	CScalableNode::Ptr pCoffHeader = NODE(
		1,false,RandColor(),CombineSize(_T("Coff Header"),m_pe.CoffHeader.GetSize()),UniqueId());

	pCoffHeader << (((
		NODE(2,false,RandColor(),_T("Machine"),_T(""),Num2String(m_pe.CoffHeader->Machine)) + 
		NODE(2,false,RandColor(),_T("NumberOfSections"),_T(""),Num2String(m_pe.CoffHeader->NumberOfSections))) * true +
		NODE(2,false,RandColor(),_T("TimeDateStamp"),_T(""),Num2String(m_pe.CoffHeader->TimeDateStamp)) + 
		NODE(2,false,RandColor(),_T("PointerToSymbolTable"),_T(""),Num2String(m_pe.CoffHeader->PointerToSymbolTable)) + 
		NODE(2,false,RandColor(),_T("NumberOfSymbols"),_T(""),Num2String(m_pe.CoffHeader->NumberOfSymbols)) + (
		NODE(2,false,RandColor(),_T("SizeOfOptionalHeader"),_T(""),Num2String(m_pe.CoffHeader->SizeOfOptionalHeader)) + 
		NODE(2,false,RandColor(),_T("Characteristics"),_T(""),Num2String(m_pe.CoffHeader->Characteristics))) * true) * false);

	CScalableNode::Ptr pOptionalHeader = NODE(
		1,false,RandColor(),CombineSize(_T("Optional Header"),m_pe.OptionalHeader.GetSize()),UniqueId());

	pOptionalHeader << (((
		NODE(2,false,RandColor(),_T("Magic"),_T(""),"'PE'")) + (
		NODE(2,false,RandColor(),_T("MajorLinkerVersion"),_T(""),Num2String(m_pe.OptionalHeader->MajorLinkerVersion)) + 
		NODE(2,false,RandColor(),_T("MinorLinkerVersion"),_T(""),Num2String(m_pe.OptionalHeader->MinorLinkerVersion))) * true) * false + 
		NODE(2,false,RandColor(),_T("SizeOfCode"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfCode)) + 
		NODE(2,false,RandColor(),_T("SizeOfInitializedData"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfInitializedData)) + 
		NODE(2,false,RandColor(),_T("SizeOfUninitializedData"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfUninitializedData)) + 
		NODE(2,false,RandColor(),_T("AddressOfEntryPoint"),_T(""),Num2String(m_pe.OptionalHeader->AddressOfEntryPoint)) +  
		NODE(2,false,RandColor(),_T("BaseOfCode"),_T(""),Num2String(m_pe.OptionalHeader->BaseOfCode)) +  
		NODE(2,false,RandColor(),_T("BaseOfData"),_T(""),Num2String(m_pe.OptionalHeader->BaseOfData)) +  
		NODE(2,false,RandColor(),_T("ImageBase"),_T(""),Num2String(m_pe.OptionalHeader->ImageBase)) +  
		NODE(2,false,RandColor(),_T("SectionAlignment"),_T(""),Num2String(m_pe.OptionalHeader->SectionAlignment)) +  
		NODE(2,false,RandColor(),_T("FileAlignment"),_T(""),Num2String(m_pe.OptionalHeader->FileAlignment)) +  (
		NODE(2,false,RandColor(),_T("MajorOperatingSystemVersion"),_T(""),Num2String(m_pe.OptionalHeader->MajorOperatingSystemVersion)) +  
		NODE(2,false,RandColor(),_T("MinorOperatingSystemVersion"),_T(""),Num2String(m_pe.OptionalHeader->MinorOperatingSystemVersion))) * false + ( 
		NODE(2,false,RandColor(),_T("MajorImageVersion"),_T(""),Num2String(m_pe.OptionalHeader->MajorImageVersion)) +  
		NODE(2,false,RandColor(),_T("MinorImageVersion"),_T(""),Num2String(m_pe.OptionalHeader->MinorImageVersion))) * false + (
		NODE(2,false,RandColor(),_T("MajorSubsystemVersion"),_T(""),Num2String(m_pe.OptionalHeader->MajorSubsystemVersion)) + 
		NODE(2,false,RandColor(),_T("MinorSubsystemVersion"),_T(""),Num2String(m_pe.OptionalHeader->MinorSubsystemVersion))) * false + 
		NODE(2,false,RandColor(),_T("Win32VersionValue"),_T(""),Num2String(m_pe.OptionalHeader->Win32VersionValue)) +  
		NODE(2,false,RandColor(),_T("SizeOfImage"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfImage)) +  
		NODE(2,false,RandColor(),_T("SizeOfHeaders"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfHeaders)) +  
		NODE(2,false,RandColor(),_T("CheckSum"),_T(""),Num2String(m_pe.OptionalHeader->CheckSum)) +  (
		NODE(2,false,RandColor(),_T("Subsystem"),_T(""),Num2String(m_pe.OptionalHeader->Subsystem)) + 
		NODE(2,false,RandColor(),_T("DllCharacteristics"),_T(""),Num2String(m_pe.OptionalHeader->DllCharacteristics))) * false + 
		NODE(2,false,RandColor(),_T("SizeOfStackReserve"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfStackReserve)) + 
		NODE(2,false,RandColor(),_T("SizeOfStackCommit"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfStackCommit)) + 
		NODE(2,false,RandColor(),_T("SizeOfHeapReserve"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfHeapReserve)) + 
		NODE(2,false,RandColor(),_T("SizeOfHeapCommit"),_T(""),Num2String(m_pe.OptionalHeader->SizeOfHeapCommit)) + 
		NODE(2,false,RandColor(),_T("LoaderFlags"),_T(""),Num2String(m_pe.OptionalHeader->LoaderFlags)) + 
		NODE(2,false,RandColor(),_T("NumberOfRvaAndSizes"),_T(""),Num2String(m_pe.OptionalHeader->NumberOfRvaAndSizes))) / 4;

	CScalableNode::Ptr pDataDirectory = NODE(
		1,false,RandColor(),CombineSize(_T("Data Directory"),sizeof(IMAGE_DATA_DIRECTORY)*IMAGE_NUMBEROF_DIRECTORY_ENTRIES),UniqueId());
	CScalableNode::Ptr pSectionTable = NODE(
		1,false,RandColor(),CombineSize(_T("Section Table"),sizeof(IMAGE_SECTION_HEADER)*m_pe.SectionHeaders.size()),UniqueId());
	CScalableNode::Ptr pSections = NODE(
		1,false,RandColor(),CombineSize(_T("Sections"),m_pe.SectionTotalSize),UniqueId());

	{
		for (auto i = m_pe.SectionHeaders.begin();
			i != m_pe.SectionHeaders.end();
			i++)
		{
			pSections->AppendChild(
				NODE(1,false,RandColor(),CombineSize(CPEFile::MultiByte2String((LPSTR)((*i)->Name),IMAGE_SIZEOF_SHORT_NAME),(*i)->SizeOfRawData),UniqueId()));
		}
	}
	
	pRoot << (
		(pDOSHeader + pDOSStub) * false +
		(pNTHeader << (
			pCoffHeader +
			pOptionalHeader +
			pDataDirectory)) +
		pSectionTable +
		pSections);

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

CDuiString CPEStructBuilder::Num2String( DWORD dwNumber )
{
	CDuiString sResult;
	sResult.Format(_T("%02X"),dwNumber);
	return sResult;
}

void CPEStructBuilder::UniqueIdInit()
{
	m_dwUniqueID = 0;
}

CDuiString CPEStructBuilder::UniqueId()
{
	CDuiString sResult;
	sResult.Format(_T("ID#%d"),m_dwUniqueID++);
	return sResult;
}