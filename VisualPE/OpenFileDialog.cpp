#include "StdAfx.h"
#include "OpenFileDialog.h"


COpenFileDialog::COpenFileDialog(HWND hwndOwner,LPCTSTR pFilter)
{
	::ZeroMemory(m_szFileName, sizeof(m_szFileName));
	::ZeroMemory(&m_ofn, sizeof(m_ofn));
	m_ofn.lStructSize = sizeof(m_ofn);
	m_ofn.hwndOwner = hwndOwner;
	m_ofn.lpstrFile = m_szFileName;
	m_ofn.nMaxFile = sizeof(m_szFileName);
	m_ofn.lpstrFilter = pFilter;
	m_ofn.nFilterIndex = 1;
	m_ofn.lpstrFileTitle = NULL;
	m_ofn.nMaxFileTitle = 0;
	m_ofn.lpstrInitialDir = _T("");
	m_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}


COpenFileDialog::~COpenFileDialog(void)
{
}

BOOL COpenFileDialog::Show()
{
	return GetOpenFileName(&m_ofn);
}

LPCTSTR COpenFileDialog::GetFileName()
{
	return m_szFileName;
}