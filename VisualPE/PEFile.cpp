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
	DWORD dwSectionOffset = DosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS);
	for (int i = 0;i < NtHeader->FileHeader.NumberOfSections;i ++)
	{
		SectionHeaders.push_back((PIMAGE_SECTION_HEADER)(FileBuf + dwSectionOffset));
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

	if (NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0 &&
		NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
	{
		return;
	}
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

void CPEFile::GetImportTable()
{
	if (NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0 &&
		NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
	{
		return;
	}
	// is not empty
	DWORD dwFileOffset = RVA2FileOffset(
		NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

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
		DWORD dwFunOffset = 0;

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

		ImportTable.push_back(importFile);
		dwFileOffset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
	}
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

void CPEFile::GetRelocationTable()
{
	ASSERT(NtHeader);

	
	if (NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0 &&
		NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0)
	{
		return;
	}

	DWORD dwOffset = RVA2FileOffset(
		NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
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
				chunk.FarAddr = *((DWORD*)(FileBuf + RVA2FileOffset(chunk.RVA)));
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
	
	if (NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress == 0 &&
		NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size == 0 )
	{
		return;
	}

	DWORD dwOffset = RVA2FileOffset(NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
	PIMAGE_RESOURCE_DIRECTORY pResource = (PIMAGE_RESOURCE_DIRECTORY)(FileBuf + dwOffset);
	
	DWORD dwCount = pResource->NumberOfIdEntries + pResource->NumberOfNamedEntries;
	for (UINT i = 0;i < dwCount;i++)
	{
		ResourceRecord resRecord;

		PIMAGE_RESOURCE_DIRECTORY_ENTRY pFirstEntry = 
			(PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD *)pResource + 
			sizeof(IMAGE_RESOURCE_DIRECTORY)/sizeof(DWORD)) + i;
			
		resRecord.Type = ResType2String(pFirstEntry->Id);
		if (pFirstEntry->DataIsDirectory == 1)
		{
			PIMAGE_RESOURCE_DIRECTORY pFirstDir = (PIMAGE_RESOURCE_DIRECTORY) ( (BYTE *)pResource + pFirstEntry->OffsetToDirectory );
			DWORD dwCount = pFirstDir->NumberOfIdEntries + pFirstDir->NumberOfNamedEntries;
			for (UINT j = 0;j < dwCount;j++)
			{
				// Second Directory Entries
				ResourceRecord::Item item;

				// get resource name or id
				PIMAGE_RESOURCE_DIRECTORY_ENTRY pSecondEntry = 
					(PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD *)pFirstDir +
					sizeof(IMAGE_RESOURCE_DIRECTORY)/sizeof(DWORD)) + j;	
				if (pSecondEntry->NameIsString == 1)
				{
					// unicode resource name string, real file offset not rva
					PIMAGE_RESOURCE_DIR_STRING_U pResDirStr = 
						(PIMAGE_RESOURCE_DIR_STRING_U)
						(FileBuf + dwOffset + pSecondEntry->NameOffset);
					item.Name.Format(_T("\"%s\""),
						(LPCTSTR)(WideChar2String(pResDirStr->NameString)));
				}
				else
				{
					// resource id
					item.Name = Number2String(pSecondEntry->Name);
				}
							
				if( pSecondEntry->DataIsDirectory == 1 )
				{
					// 3rd directory
					PIMAGE_RESOURCE_DIRECTORY pThirdEntry = 
						(PIMAGE_RESOURCE_DIRECTORY)
						(FileBuf + dwOffset + pSecondEntry->OffsetToDirectory);
					if( pThirdEntry->NumberOfIdEntries + pThirdEntry->NumberOfNamedEntries == 1 )
					{
						// Entry exist
						PIMAGE_RESOURCE_DIRECTORY_ENTRY pThirdEntry = 
							(PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD *)pSecondEntry +
							sizeof(IMAGE_RESOURCE_DIRECTORY) / sizeof(DWORD));	
						if ( pThirdEntry->DataIsDirectory == 0 )
						{
							PIMAGE_RESOURCE_DATA_ENTRY pData = 
								(PIMAGE_RESOURCE_DATA_ENTRY)
								(FileBuf + dwOffset + pThirdEntry->OffsetToDirectory);
							if (pData)
							{
								item.RVA = pData->OffsetToData;
								item.Size = pData->Size;
							}										
						}
					}
				}
				resRecord.Items.push_back(item);
			}
		}
		ResourceTable.push_back(resRecord);
	}
}

CDuiString CPEFile::MultiByte2String(LPCSTR pSource) const
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
CDuiString CPEFile::WideChar2String( LPCWSTR pSource ) const
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
	dwLen = WideCharToMultiByte (CP_ACP, 0, pSource, NULL, NULL, 0, "?", &bFlag);
	scoped_ptr<char> pBuf(new char[dwLen]);
	if (pBuf)
	{
		WideCharToMultiByte (CP_ACP, 0, pSource, NULL, pBuf.get(), dwLen, "?", &bFlag);
		sResult = pBuf.get();
	}
#endif
	return sResult;
}

CDuiString CPEFile::Number2String( DWORD dwNumber ) const
{
	CDuiString sResult;
	sResult.Format(_T("0x%Xd"),dwNumber);

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