#include "StdAfx.h"
#include "PEFile.h"


CPEFile::CPEFile(void)
{
}


CPEFile::~CPEFile(void)
{
}

bool CPEFile::LoadFile( CDuiString sFilePath )
{
	HANDLE hFile = ::CreateFile(
		sFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	if(!::ReadFile(hFile,(void*)&DOSHeader,sizeof(DOSHeader),NULL,NULL))
	{
		return false;
	}

	DOSStub = new BYTE[DOSHeader.e_lfanew - sizeof(DOSHeader)];
	::SetFilePointer(hFile,DOSHeader.e_lfanew,NULL,FILE_BEGIN);

	if (!::ReadFile(hFile,(void*)&NTHeader,sizeof(NTHeader),NULL,NULL))
	{
		return false;
	}


	return true;
}