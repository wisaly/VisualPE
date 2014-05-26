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
	COLORREF RandColor();
private:
	CPEFile &m_pe;
	//variate_generator <mt19937&,uniform_int<>>m_rand;
};

