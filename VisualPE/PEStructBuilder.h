#pragma once
#include "ScalableNode.h"
#include "PEFile.h"

class CPEStructBuilder
{
public:
	CPEStructBuilder(CPEFile &pe);
	~CPEStructBuilder(void);

	CScalableNode::Ptr pResult;
	int nMaxLevel;

private:
	void Build();
	DWORD RandColor();
	CDuiString CombineSize(CDuiString sDescription,DWORD dwSize);
	CDuiString Size2String(DWORD dwSize);
	CDuiString Num2String(DWORD dwNumber);
	void UniqueIdInit();
	CDuiString UniqueId();
private:
	CPEFile &m_pe;
	DWORD m_dwUniqueID;
	//variate_generator <mt19937&,uniform_int<>>m_rand;
};

