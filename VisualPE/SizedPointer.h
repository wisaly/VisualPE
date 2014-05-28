#pragma once

#pragma region Policy Template
template<class Type>
class NotManage
{
public:
	Type* Allocate()
	{
		return 0;
	}
protected:
	void Release(Type*)
	{
	}
};

template<class Type>
class HeapManage
{
public:
	HeapManage()
		:m_pManaged(0)
	{

	}
	Type* Allocate()
	{
		Release();
		m_pManaged = new Type;
		::ZeroMemory(m_pManaged,sizeof(Type));
		Type *&pObj = RefPt();
		pObj = m_pManaged;
		return m_pManaged;
	}
	~HeapManage()
	{
		Release();
	}
protected:
	// save allocated memory to avoid memory leak
	// because CSizedPointer allow to change pointer value
	Type* m_pManaged;
	void Release()
	{
		if (m_pManaged)
		{
			delete m_pManaged;
		}
	}
	virtual Type *&RefPt() = 0;
};

template<class Type>
class HeapArrayManage
{
public:
	HeapArrayManage()
		:m_pManaged(0)
	{

	}
	void Allocate(int nCount)
	{
		Release();
		m_pManaged = new Type[nCount];
		::ZeroMemory(m_pManaged,sizeof(Type)*nCount);

		Type *&pObj = RefPt();
		pObj = m_pManaged;
		int &sz = RefSz();
		sz = sizeof(Type) * nCount;
	}
	virtual ~HeapArrayManage()
	{
		Release();
	}
protected:
	void Release()
	{
		if (m_pManaged)
		{
			delete m_pManaged;
		}
	}
	virtual Type *&RefPt() = 0;
	virtual int &RefSz() = 0;
	Type* m_pManaged;
};

#pragma endregion

template
	<class Type,
	template <class> class ManagePolicy = NotManage
	>
class CSizedPointer : public ManagePolicy<Type>
{
public:
	CSizedPointer()
		:m_nSize(sizeof(Type))
	{
	}
	~CSizedPointer(void)
	{
	}

	Type* operator ->()
	{
		return m_pObj;
	}

	Type& operator *()
	{
		ASSERT( m_pObj != 0 );
		return *m_pObj;
	}

	// convert to original type
	operator Type*()
	{
		return m_pObj;
	}

	// force convert to any type
	template<class U>
	operator U*()
	{
		return (U*)m_pObj;
	}

	// support array like operation
	Type & operator[](int i) const
	{
		ASSERT( m_pObj != 0 );
		ASSERT( i >= 0 );
		return m_pObj[i];
	}

	// support pointer offset
	Type *operator +(int nOffset)
	{
		return m_pObj + nOffset;
	}

	// support change value ( for NotManage policy )
	CSizedPointer &operator =(Type* pObj)
	{
		m_pObj = pObj;
		m_nSize = sizeof(Type);

		return *this;
	}

	// manual set size
	void SetSize(int dwSize)
	{
		m_nSize = dwSize;
	}
	// get size
	int GetSize() const
	{
		return m_nSize;
	}

// private:
// 	// support boolean convert
// 	typedef Type * CSizedPointer::*unspecified_bool_type;
// public:
// 	operator unspecified_bool_type() const
// 	{
// 		return m_pObj == 0? 0: &CSizedPointer::m_pObj;
// 	}

protected:
	// 
	Type *&RefPt()
	{
		return m_pObj;
	}
	int &RefSz()
	{
		return m_nSize;
	}
private:
	int m_nSize;
	Type *m_pObj;
};

