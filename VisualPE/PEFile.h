#pragma once
#include "SizedPointer.h"

struct ImportFile
{
	CDuiString FileName;
	vector<CDuiString> Functions;
};

struct ExportFile
{
	struct Function
	{
		CDuiString Name;
		CDuiString RVA;
		CDuiString Index;
	};

	CDuiString FileName;
	vector<Function> Functions;
	DWORD NamedFunCount;
};

class CPEFile
{
public:
	CPEFile(void);
	~CPEFile(void);
	
	CSizedPointer<BYTE,HeapArrayManage> FileBuf;

	CSizedPointer<IMAGE_DOS_HEADER> DosHeader;
	CSizedPointer<BYTE> DosStub;
	CSizedPointer<IMAGE_NT_HEADERS> NtHeader;
	vector<PIMAGE_SECTION_HEADER> SectionHeaders;
	vector<ImportFile> ImportFiles;

	bool LoadFile(CDuiString sFilePath);

	DWORD RVA2FileOffset(DWORD dwRVA) const;
	CDuiString MultiByte2String(LPCSTR pSource) const;
	CDuiString Number2String(DWORD dwNumber) const;

private:
	bool m_bIs64;
};

