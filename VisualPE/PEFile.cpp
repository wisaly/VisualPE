#include "StdAfx.h"
#include "PEFile.h"


CPEFile::CPEFile(void)
	:pFileBuf(0)
{
}


CPEFile::~CPEFile(void)
{
	if (pFileBuf)
	{
		delete pFileBuf;
	}
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

	DWORD dwFileSizeHigh = 0;
	pFileBuf.SetSize(::GetFileSize(hFile,&dwFileSizeHigh));

	if(dwFileSizeHigh > 0)
	{
		return false;
	}

	pFileBuf = new BYTE[pFileBuf.GetSize()];
	if (!pFileBuf)
	{
		return false;
	}

	DWORD dwReadSize = 0;
	if (!::ReadFile(hFile,pFileBuf,pFileBuf.GetSize(),&dwReadSize,NULL))
	{
		return false;
	}

	if (dwReadSize != pFileBuf.GetSize())
	{
		return false;
	}

	::CloseHandle(hFile);

	// DOS header
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuf;

	// DOS stub
	pDosStub.SetSize(pDosHeader->e_lfanew - sizeof(IMAGE_DOS_HEADER));
	pDosStub = pFileBuf + sizeof(IMAGE_DOS_HEADER);

	// PE header
	pNtHeader = (PIMAGE_NT_HEADERS)(pFileBuf + (DWORD)pDosHeader->e_lfanew);

	// 

	return true;
}