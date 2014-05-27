#include "StdAfx.h"
#include "PEFile.h"


CPEFile::CPEFile(void)
{
}


CPEFile::~CPEFile(void)
{
// 	if (FileBuf)
// 	{
// 		delete FileBuf;
// 	}
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
	DWORD dwFileSize = ::GetFileSize(hFile,&dwFileSizeHigh);

	if(dwFileSizeHigh > 0)
	{
		return false;
	}

	FileBuf.Allocate(dwFileSize);
	if (FileBuf)
	{
		return false;
	}

	DWORD dwReadSize = 0;
	if (!::ReadFile(hFile,FileBuf,FileBuf.GetSize(),&dwReadSize,NULL))
	{
		return false;
	}

	if (dwReadSize != FileBuf.GetSize())
	{
		return false;
	}

	::CloseHandle(hFile);

	// DOS header
	DosHeader = (PIMAGE_DOS_HEADER)FileBuf;

	// DOS stub
	DosStub.SetSize(DosHeader->e_lfanew - sizeof(IMAGE_DOS_HEADER));
	DosStub = FileBuf + sizeof(IMAGE_DOS_HEADER);

	// PE header
	NtHeader = (PIMAGE_NT_HEADERS)(FileBuf + (DWORD)DosHeader->e_lfanew);

	// check valid
	if(IMAGE_DOS_SIGNATURE != (DWORD)DosHeader->e_magic
		|| IMAGE_NT_SIGNATURE != (DWORD)NtHeader->Signature)
	{
		return false;
	}
	
	// check 64 bit
	m_bIs64 = NtHeader->FileHeader.Machine ==  IMAGE_FILE_MACHINE_IA64 
		|| NtHeader->FileHeader.Machine ==  IMAGE_FILE_MACHINE_AMD64 ;

	// section header
	SectionHeaders.clear();

	DWORD dwSectionOffset = DosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS);
	for (int i = 0;i < NtHeader->FileHeader.NumberOfSections;i ++)
	{
		SectionHeaders.push_back((PIMAGE_SECTION_HEADER)(FileBuf + dwSectionOffset));
	}

	// import table
	if ( NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress != 0 ||
		NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0 )
	{
		// is not empty
		DWORD dwFileOffset = RVA2FileOffset(
			NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		for(;;)
		{
			ImportFile importFile;
			DWORD dwFunOffset = 0;

			PIMAGE_IMPORT_DESCRIPTOR pIID =(PIMAGE_IMPORT_DESCRIPTOR)(FileBuf + dwFileOffset);
			if ( pIID->FirstThunk == NULL && 
				pIID->ForwarderChain==NULL &&
				pIID->Name == NULL && 
				pIID->OriginalFirstThunk == NULL && 
				pIID->TimeDateStamp == NULL)
			{
				// import end
				break;
			}
						
			importFile.FileName = MultiByte2String(
				(LPCSTR)(FileBuf + RVA2FileOffset(pIID->Name)));
			// get functions
			if( pIID->OriginalFirstThunk == 0 )
			{
				// FirstThunk
				DWORD dwThunkOffset = RVA2FileOffset( pIID->FirstThunk );
				for(;;)
				{
					PIMAGE_THUNK_DATA pITD = (PIMAGE_THUNK_DATA)(FileBuf + dwThunkOffset);

					if ( pITD->u1.Function == NULL )
					{
						// end of this file
						break;
					}

					dwFunOffset = RVA2FileOffset( pITD->u1.Function );
					importFile.Functions.push_back(MultiByte2String(
						(LPCSTR)(((PIMAGE_IMPORT_BY_NAME)
						(FileBuf + dwFunOffset))->Name)) );

					dwThunkOffset += sizeof(IMAGE_THUNK_DATA);
				}
			}
			else
			{
				// OriginalFirstThunk
				DWORD dwThunkOffset = RVA2FileOffset( pIID->OriginalFirstThunk );
				for(;;)
				{
					PIMAGE_THUNK_DATA pITD = (PIMAGE_THUNK_DATA)(FileBuf + dwThunkOffset);

					if (pITD->u1.Function == NULL)
					{
						// end of this file
						break;
					}
					DWORD dwMask = m_bIs64 ? 
						(DWORD)((pITD->u1.Function & IMAGE_ORDINAL_FLAG64) >> 63) : 
						(DWORD)((pITD->u1.Function & IMAGE_ORDINAL_FLAG32) >> 31);

					if (dwMask == 1)
					{
						// import by id
						DWORD dwID;
						dwID = m_bIs64 ?
							(DWORD)(pITD->u1.Function & (~IMAGE_ORDINAL_FLAG64)) :
							(DWORD)(pITD->u1.Function & (~IMAGE_ORDINAL_FLAG32));

						importFile.Functions.push_back(
							CDuiString(_T("FunID: ")) + Number2String(dwID));
					}
					else
					{
						// import by name
						dwFunOffset = RVA2FileOffset( pITD->u1.Function );
						importFile.Functions.push_back(
							MultiByte2String(
							(LPCSTR)(((PIMAGE_IMPORT_BY_NAME)
							(FileBuf + dwFunOffset))->Name)));
					}

					dwThunkOffset += sizeof(IMAGE_THUNK_DATA);
				}

			}

			ImportFiles.push_back(importFile);
			dwFileOffset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
		}
	}

	// Export table
	if (NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress != 0 ||
		NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size != 0)
	{
		PIMAGE_EXPORT_DIRECTORY pIED = (PIMAGE_EXPORT_DIRECTORY)(FileBuf + 
			NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

		ExportFile exportFile;
		if ( pIED->Name )
		{
			// file name
			exportFile.FileName = MultiByte2String(
				(LPCSTR)(FileBuf + RVA2FileOffset(pIED->Name)));
		}

		exportFile.NamedFunCount = pIED->NumberOfNames;
		
		for (UINT i=0;i < pIED->NumberOfFunctions;i ++)
		{
			WORD wOrdinals = 0;
			DWORD dwOffsetFun = RVA2FileOffset(pIED->AddressOfFunctions);
			DWORD dwFunRVA = *((DWORD *)(FileBuf + dwOffsetFun + (i << 2)));
			if ( dwFunRVA != 0 )
			{
				ExportFile::Function exportFun;

				for ( UINT j=0 ; j < pIED->NumberOfNames; j++)
				{
					// is function have name
					DWORD dwNameOffset = RVA2FileOffset(
						pIED->AddressOfNames + (j << 2));
					dwNameOffset = 
						RVA2FileOffset(*((DWORD *)(FileBuf + dwNameOffset)));
					
					DWORD dwOffsetOrdinal = RVA2FileOffset(
						pIED->AddressOfNameOrdinals + (j << 1));
					wOrdinals = *((WORD *)(FileBuf + dwOffsetOrdinal));

					if ( wOrdinals == i)
					{
						exportFun.Name = MultiByte2String(
							(LPCSTR)(FileBuf + dwNameOffset));
						break;
					}						
				}

				exportFun.Index = Number2String (i + pIED->Base);
				exportFun.RVA = Number2String(dwFunRVA);
				exportFile.Functions.push_back(exportFun);
			}
		}
	}

	return true;
}

DWORD CPEFile::RVA2FileOffset( DWORD dwRVA ) const
{
	for (auto i = SectionHeaders.begin();
		i != SectionHeaders.end();
		i++)
	{
		if ((*i)->VirtualAddress <= dwRVA &&
			(*i)->VirtualAddress + (*i)->Misc.VirtualSize >= dwRVA)
		{
			return dwRVA - 
				((*i)->VirtualAddress - (*i)->PointerToRawData);
		}
	}

	return 0;
}

CDuiString CPEFile::MultiByte2String( LPCSTR pSource ) const
{
	CDuiString sResult;
	if (pSource == NULL)
	{
		return sResult;
	}

#ifdef _UNICODE
	DWORD dwLen;
	dwLen = ::MultiByteToWideChar (CP_ACP, 0, pSource, -1, NULL, 0);
	scoped_ptr<wchar_t> pBuf(new wchar_t[dwLen]);
	if ( pBuf )
	{
		//::ZeroMemory(pBuf, dwLen);
		::MultiByteToWideChar (CP_ACP, 0, pSource, -1, pBuf.get(), dwLen); 
		sResult = pBuf.get();
	}
#else
	sResult = pSource;
#endif

	return sResult;
}

CDuiString CPEFile::Number2String( DWORD dwNumber ) const
{
	CDuiString sResult;
	sResult.Format(_T("0x%Xd"),dwNumber);

	return sResult;
}