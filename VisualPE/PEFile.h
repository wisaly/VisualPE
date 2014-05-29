#pragma once
#include "SizedPointer.h"

struct ImportFile
{
	struct Function
	{
		CDuiString Name;
		DWORD Id;
		DWORD RVA;
	};
	CDuiString FileName;
	vector<Function> Functions;
};

struct ExportFile
{
	struct Function
	{
		CDuiString Name;
		DWORD RVA;
		DWORD Index;
	};

	CDuiString FileName;
	vector<Function> Functions;
	DWORD NamedFunCount;
};

struct RelocationItem
{
	struct Chunk
	{
		UINT Index;
		DWORD RVA;
		DWORD FarAddr;
		CDuiString Type;
	};

	UINT Index;
	vector<Chunk> Chunks;
	CDuiString SectionName;
};

struct ResourceRecord
{
	struct Entry
	{
		WORD Id;
		CDuiString Name;
		DWORD RVA;
		DWORD Size;
	};
	CDuiString Type;
	vector<Entry> Entries;
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
	CSizedPointer<IMAGE_FILE_HEADER> CoffHeader;
	CSizedPointer<IMAGE_OPTIONAL_HEADER> OptionalHeader;
	vector<PIMAGE_SECTION_HEADER> SectionHeaders;
	DWORD SectionTotalSize;
	vector<ExportFile> ExportTable;
	vector<ImportFile> ImportTable;
	vector<ResourceRecord> ResourceTable;
	vector<RelocationItem> RelocationTable;

	bool LoadFile(CDuiString sFilePath);

	DWORD RVA2FOA(DWORD dwRVA) const;
	static CDuiString MultiByte2String(LPCSTR pSource,int cbSource = -1);
	static CDuiString WideChar2String(LPCWSTR pSource,int cbSource = -1);
	CDuiString Number2String(DWORD dwNumber) const;
	CDuiString ResType2String(DWORD dwType) const;

	CDuiString GetSectionName(DWORD dwRVA);

private:
	void GetExportTable();
	void GetImportTable();
	void GetResourseTable();
	void GetExceptionTable();
	void GetCertificateTable();
	void GetRelocationTable();

private:
	bool m_bIs64;
};

