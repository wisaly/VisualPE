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
	if (!FileBuf)
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
	DosStub = FileBuf + sizeof(IMAGE_DOS_HEADER);
	DosStub.SetSize(DosHeader->e_lfanew - sizeof(IMAGE_DOS_HEADER));

	// PE header
	NtHeader = (PIMAGE_NT_HEADERS)(FileBuf + (DWORD)DosHeader->e_lfanew);
	CoffHeader = &NtHeader->FileHeader;
	OptionalHeader = &NtHeader->OptionalHeader;

	// check valid
	DWORD d1 = (DWORD)DosHeader->e_magic;
	DWORD d2 = (DWORD)NtHeader->Signature;
	if(IMAGE_DOS_SIGNATURE != (DWORD)DosHeader->e_magic
		|| IMAGE_NT_SIGNATURE != (DWORD)NtHeader->Signature)
	{
		return false;
	}
	
	// check 64 bit
	m_bIs64 = CoffHeader->Machine ==  IMAGE_FILE_MACHINE_IA64 
		|| NtHeader->FileHeader.Machine ==  IMAGE_FILE_MACHINE_AMD64 ;

	// section header
	DWORD dwSectionOffset = DosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS);
	SectionTotalSize = 0;
	for (int i = 0;i < NtHeader->FileHeader.NumberOfSections;i ++)
	{
		PIMAGE_SECTION_HEADER pHeader = 
			(PIMAGE_SECTION_HEADER)(FileBuf + dwSectionOffset);
		SectionHeaders.push_back(pHeader);
		dwSectionOffset += sizeof(IMAGE_SECTION_HEADER);

		SectionTotalSize += pHeader->SizeOfRawData;
	}

	GetExportTable();
	GetImportTable();
	GetResourseTable();
	GetRelocationTable();


	return true;
}

void CPEFile::GetExportTable()
{
	ASSERT(NtHeader);

	if (OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0 &&
		OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
	{
		return;
	}

	DWORD dwOffset = OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	if (dwOffset == 0)
	{
		return;
	}
	PIMAGE_EXPORT_DIRECTORY pIED = (PIMAGE_EXPORT_DIRECTORY)(FileBuf + 
		dwOffset);

	ExportFile exportFile;
	if ( pIED->Name )
	{
		// file name
		exportFile.FileName = MultiByte2String(
			(LPCSTR)(FileBuf + RVA2FOA(pIED->Name)));
	}

	exportFile.NamedFunCount = pIED->NumberOfNames;

	for (UINT i=0;i < pIED->NumberOfFunctions;i ++)
	{
		WORD wOrdinals = 0;
		DWORD dwOffsetFun = RVA2FOA(pIED->AddressOfFunctions);
		DWORD dwFunRVA = *((DWORD *)(FileBuf + dwOffsetFun + (i << 2)));
		if ( dwFunRVA != 0 )
		{
			ExportFile::Function exportFun;

			for ( UINT j=0 ; j < pIED->NumberOfNames; j++)
			{
				// is function have name
				DWORD dwNameOffset = RVA2FOA(
					pIED->AddressOfNames + (j << 2));
				dwNameOffset = 
					RVA2FOA(*((DWORD *)(FileBuf + dwNameOffset)));

				DWORD dwOffsetOrdinal = RVA2FOA(
					pIED->AddressOfNameOrdinals + (j << 1));
				wOrdinals = *((WORD *)(FileBuf + dwOffsetOrdinal));

				if ( wOrdinals == i)
				{
					exportFun.Name = MultiByte2String(
						(LPCSTR)(FileBuf + dwNameOffset));
					break;
				}						
			}

			exportFun.Index = i + pIED->Base;
			exportFun.RVA = dwFunRVA;
			exportFile.Functions.push_back(exportFun);
		}
	}
}

void CPEFile::GetImportTable()
{
	if (OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0 &&
		OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
	{
		return;
	}
	// is not empty
	DWORD dwFileOffset = RVA2FOA(
		OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	if (dwFileOffset == 0)
	{
		return ;
	}

	for(;;)
	{
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

		ImportFile importFile;

		importFile.FileName = MultiByte2String(
			(LPCSTR)(FileBuf + RVA2FOA(pIID->Name)));
		// get functions
		DWORD dwThunkOffset = pIID->OriginalFirstThunk != 0 ?
			RVA2FOA(pIID->OriginalFirstThunk) : 
			RVA2FOA(pIID->FirstThunk);

		for(;;)
		{
			PIMAGE_THUNK_DATA pITD = (PIMAGE_THUNK_DATA)(FileBuf + dwThunkOffset);

			if (pITD->u1.Function == NULL)
			{
				// end of this file
				break;
			}

			ImportFile::Function func;

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

				func.Id = dwID;
			}
			else
			{
				// import by name
				DWORD dwFunOffset = RVA2FOA(pITD->u1.Function);

				func.Name = MultiByte2String(
					(LPCSTR)(((PIMAGE_IMPORT_BY_NAME)
					(FileBuf + dwFunOffset))->Name));
			}
			importFile.Functions.push_back(func);

			dwThunkOffset += sizeof(IMAGE_THUNK_DATA);
		}

		ImportTable.push_back(importFile);
		dwFileOffset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
	}
}

DWORD CPEFile::RVA2FOA( DWORD dwRVA ) const
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

void CPEFile::GetRelocationTable()
{
	ASSERT(NtHeader);

	
	if (OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0 &&
		OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0)
	{
		return;
	}

	DWORD dwOffset = RVA2FOA(
		OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	if (dwOffset == 0)
	{
		return;
	}

	PIMAGE_BASE_RELOCATION pReloc = (PIMAGE_BASE_RELOCATION)(FileBuf + dwOffset );

	UINT nIndex = 1;
	while (pReloc->VirtualAddress)
	{
		RelocationItem relocItem;
		DWORD *pItem = (DWORD *)pReloc + 2;
		
		relocItem.Index = nIndex;
		relocItem.SectionName = GetSectionName(pReloc->VirtualAddress);
		DWORD dwCount = (pReloc->SizeOfBlock - (sizeof(DWORD) << 1 ))
			/ sizeof(WORD);// count of TypeOffset

		for (UINT i = 0;i < dwCount;i++)
		{
			RelocationItem::Chunk chunk;
			WORD wOffset = (WORD)(*pItem);

			switch((wOffset & 0xF000) >> 12 )
			{
			case IMAGE_REL_BASED_ABSOLUTE:
				chunk.Type = _T("ABSOLUTE(0)");
				break;
			case IMAGE_REL_BASED_HIGH:
				chunk.Type = _T("HIGH(1)");
				break;
			case IMAGE_REL_BASED_LOW:
				chunk.Type = _T("LOW(2)");
				break;
			case IMAGE_REL_BASED_HIGHLOW:
				chunk.Type = _T("HIGHLOW(3)");
				break;
			case IMAGE_REL_BASED_HIGHADJ:
				chunk.Type = _T("HIGHADJ(4)");
				break;
			case IMAGE_REL_BASED_MIPS_JMPADDR:
				chunk.Type = _T("JMPADDR(5)");
				break;
			/*case IMAGE_REL_BASED_MIPS_JMPADDR16:
				RC.type = _T("JMPADDR16(9)");
				break;*/
			case IMAGE_REL_BASED_IA64_IMM64:
				chunk.Type = _T("IMM64(9)");
				break;
			case IMAGE_REL_BASED_DIR64:
				chunk.Type = _T("DIR64(10)");
				break;
			}

			if( wOffset != 0 )
			{
				chunk.RVA = pReloc->VirtualAddress+(wOffset & 0x0FFF);
				chunk.FarAddr = *((DWORD*)(FileBuf + RVA2FOA(chunk.RVA)));
			}
			else
			{
				// end
				chunk.RVA = 0;
				chunk.FarAddr = 0;
			}
			chunk.Index = i + 1;

			relocItem.Chunks.push_back(chunk);

			pItem =(DWORD *)((WORD*)pItem + 1);// move to next Item				
		}
		RelocationTable.push_back(relocItem);

		pReloc=(PIMAGE_BASE_RELOCATION)((DWORD)pReloc + pReloc->SizeOfBlock);			
	}
}

void CPEFile::GetResourseTable()
{
	ASSERT(NtHeader);
	
	if (OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress == 0 &&
		OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size == 0 )
	{
		return;
	}

	DWORD dwOffset = RVA2FOA(OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
	if (dwOffset == 0)
	{
		return;
	}
	PIMAGE_RESOURCE_DIRECTORY pResource = (PIMAGE_RESOURCE_DIRECTORY)(FileBuf + dwOffset);
	
	DWORD dwCount = pResource->NumberOfIdEntries + pResource->NumberOfNamedEntries;
	for (UINT i = 0;i < dwCount;i++)
	{
		ResourceRecord resRecord;

		PIMAGE_RESOURCE_DIRECTORY_ENTRY pFirstEntry = 
			(PIMAGE_RESOURCE_DIRECTORY_ENTRY)
			((BYTE *)pResource + sizeof(IMAGE_RESOURCE_DIRECTORY))
			+ i;
		
		if (!pFirstEntry->NameIsString)
		{
			resRecord.Type = ResType2String(pFirstEntry->Id);
		}

		if (pFirstEntry->DataIsDirectory == 1)
		{
			PIMAGE_RESOURCE_DIRECTORY pFirstDir = (PIMAGE_RESOURCE_DIRECTORY)((BYTE *)pResource + pFirstEntry->OffsetToDirectory);
			DWORD dwCount = pFirstDir->NumberOfIdEntries + pFirstDir->NumberOfNamedEntries;
			for (UINT j = 0;j < dwCount;j++)
			{
				// Second Directory Entries
				ResourceRecord::Entry entry;

				// get resource name or id
				PIMAGE_RESOURCE_DIRECTORY_ENTRY pSecondEntry = 
					(PIMAGE_RESOURCE_DIRECTORY_ENTRY)
					((BYTE *)pFirstDir + sizeof(IMAGE_RESOURCE_DIRECTORY))
					+ j;

				if (pSecondEntry->NameIsString == 1)
				{
					// unicode resource name string, real file offset not rva
					PIMAGE_RESOURCE_DIR_STRING_U pResDirStr = 
						(PIMAGE_RESOURCE_DIR_STRING_U)
						(FileBuf + dwOffset + pSecondEntry->NameOffset);
					entry.Name = WideChar2String(pResDirStr->NameString);
				}
				else
				{
					// resource id
					entry.Id = pSecondEntry->Id;
				}

				if( pSecondEntry->DataIsDirectory == 1 )
				{
					// 2nd directory
					PIMAGE_RESOURCE_DIRECTORY pSecondDir = 
						(PIMAGE_RESOURCE_DIRECTORY)
						(FileBuf + dwOffset + pSecondEntry->OffsetToDirectory);
					if( pSecondDir->NumberOfIdEntries + pSecondDir->NumberOfNamedEntries == 1 )
					{
						// Entry exist
						PIMAGE_RESOURCE_DIRECTORY_ENTRY pThirdEntry = 
							(PIMAGE_RESOURCE_DIRECTORY_ENTRY)((BYTE *)pSecondDir +
							sizeof(IMAGE_RESOURCE_DIRECTORY));	
						if ( pThirdEntry->DataIsDirectory == 0 )
						{
							PIMAGE_RESOURCE_DATA_ENTRY pData = 
								(PIMAGE_RESOURCE_DATA_ENTRY)
								(FileBuf + dwOffset + pThirdEntry->OffsetToData);
							if (pData)
							{
								entry.RVA = pData->OffsetToData;
								entry.Size = pData->Size;
							}										
						}
					}
				}
				resRecord.Entries.push_back(entry);
			}
		}
		ResourceTable.push_back(resRecord);
	}
}

CDuiString CPEFile::MultiByte2String(LPCSTR pSource,int cbSource)
{
	CDuiString sResult;
	if (pSource == NULL)
	{
		return sResult;
	}

#ifdef _UNICODE
	DWORD dwLen;
	dwLen = ::MultiByteToWideChar (CP_ACP, 0, pSource, cbSource, NULL, 0);
	CSizedPointer<wchar_t,HeapArrayManage> pBuf;
	pBuf.Allocate(dwLen + 1);
	if ( pBuf )
	{
		::ZeroMemory(pBuf, pBuf.GetSize());
		::MultiByteToWideChar (CP_ACP, 0, pSource, cbSource, pBuf, dwLen); 
		sResult = (LPCTSTR)pBuf;
	}
#else
	sResult = pSource;
#endif

	return sResult;
}
CDuiString CPEFile::WideChar2String( LPCWSTR pSource,int cbSource )
{
	CDuiString sResult;
	if (pSource == NULL)
	{
		return sResult;
	}

#ifdef _UNICODE
	sResult = pSource;
#else
	DWORD dwLen;
	BOOL bFlag;
	dwLen = WideCharToMultiByte (CP_ACP, 0, pSource, cbSource, NULL, 0, "?", &bFlag);
	scoped_ptr<char> pBuf(new char[dwLen + 1]);
	if (pBuf)
	{
		::ZeroMemory(pBuf.get(), dwLen);
		WideCharToMultiByte (CP_ACP, 0, pSource, cbSource, pBuf.get(), dwLen, "?", &bFlag);
		sResult = pBuf.get();
	}
#endif
	return sResult;
}

CDuiString CPEFile::Number2String( DWORD dwNumber ) const
{
	CDuiString sResult;
	sResult.Format(_T("0x%X"),dwNumber);

	return sResult;
}

CDuiString CPEFile::GetSectionName( DWORD dwRVA )
{
	CDuiString sResult;
	
	for(auto i = SectionHeaders.begin();
		i != SectionHeaders.end();
		i++)
	{
		if((*i)->VirtualAddress <= dwRVA && 
			(*i)->VirtualAddress + (*i)->Misc.VirtualSize >= dwRVA	)
		{
			sResult = MultiByte2String((LPCSTR)((*i)->Name));
			break;
		}
	}
	return sResult;
}

CDuiString CPEFile::ResType2String( DWORD dwType ) const
{
	switch(dwType)
	{
	case RT_CURSOR:
		return _T("Cursor");
	case RT_BITMAP:
		return _T("Bitmap");
	case RT_ICON:
		return _T("Icon");
	case RT_MENU:
		return _T("Menu");
	case RT_DIALOG:
		return _T("Dialog");
	case RT_STRING:
		return _T("String");
	case RT_FONTDIR:
		return _T("Font Directory");
	case RT_FONT:
		return _T("Font");
	case RT_ACCELERATOR:
		return _T("Accelerator");
	case RT_RCDATA:
		return _T("Resource Data");
	case RT_MESSAGETABLE:
		return _T("Message Table");
	case RT_GROUP_CURSOR:
		return _T("Cursor Group");
	case RT_GROUP_ICON:
		return _T("Icon Group");
	case RT_VERSION:
		return _T("Version");
	case RT_DLGINCLUDE:
		return _T("Dialog Include");
	case RT_PLUGPLAY:
		return _T("Plug Play");
	case RT_VXD:
		return _T("VXD");
	case RT_ANICURSOR:
		return _T("Animation Cursor");
	case RT_ANIICON:
		return _T("Animation Icon");
	case RT_HTML:
		return _T("HTML");
	case RT_MANIFEST:
		return _T("Manifest");
	default:
		return _T("Custom Resource");
	}
}