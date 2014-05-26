#pragma once
class CPEFile
{
public:
	CPEFile(void);
	~CPEFile(void);

	IMAGE_DOS_HEADER DOSHeader;
	BYTE *DOSStub;
	IMAGE_NT_HEADERS32 NTHeader;
	IMAGE_SECTION_HEADER SectionHeader;

	bool LoadFile(CDuiString sFilePath);
};

