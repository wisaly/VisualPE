#pragma once
#include "SizedPointer.h"
class CPEFile
{
public:
	CPEFile(void);
	~CPEFile(void);

	CSizedPointer<BYTE> pFileBuf;

	CSizedPointer<IMAGE_DOS_HEADER> pDosHeader;
	CSizedPointer<BYTE> pDosStub;
	CSizedPointer<IMAGE_NT_HEADERS32> pNtHeader;
	CSizedPointer<IMAGE_SECTION_HEADER> pSectionHeader;

	bool LoadFile(CDuiString sFilePath);
};

