#pragma once

template<class T>
class CSizedPointer
{
public:
	CSizedPointer(T* pObj= 0)
		:m_pObj(pObj),m_dwSize(sizeof(T))
	{
	}
	~CSizedPointer(void)
	{

	}

	T* operator ->()
	{
		return m_pObj;
	}

	T& operator *()
	{
		return *m_pObj;
	}

	operator T*()
	{
		return m_pObj;
	}

	CSizedPointer &operator =(CSizedPointer &pOther)
	{
		m_pObj = pOther->m_pObj;
		m_dwSize = pOther->m_dwSize;
	}
	CSizedPointer &operator =(T* pObj)
	{
		m_pObj = pObj;
		m_dwSize = 0;
	}

	void SetSize(DWORD dwSize)
	{
		m_dwSize = dwSize;
	}
	DWORD GetSize() const
	{
		return m_dwSize;
	}

private:
	DWORD m_dwSize;
	T *m_pObj;
};

