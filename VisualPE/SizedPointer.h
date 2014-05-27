#pragma once

#pragma region Policy Template
template<class T>
class NotManage
{
public:
	T* Allocate()
	{
		return 0;
	}
protected:
	void Release(T*)
	{
	}
};

template<class T>
class HeapManage
{
public:
	T* Allocate()
	{
		Release();
		m_pManaged = new T;
		::ZeroMemory(m_pManaged,sizeof(T));
		return m_pManaged;
	}
	~HeapManage()
	{
		Release();
	}
protected:
	// save allocated memory to avoid memory leak
	// because CSizedPointer allow to change pointer value
	T* m_pManaged;
	void Release()
	{
		if (m_pManaged)
		{
			delete m_pManaged;
		}
	}
};

template<class T>
class HeapArrayManage
{
public:
	T* Allocate(int nCount)
	{
		Release();
		m_pManaged = new T[nCount];
		::ZeroMemory(m_pManaged,sizeof(T)*nCount);
		return m_pManaged;
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
	T* m_pManaged;
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

private:
	// support boolean convert
	typedef Type * CSizedPointer::*unspecified_bool_type;
	operator unspecified_bool_type() const
	{
		return m_pObj == 0? 0: &CSizedPointer::m_pObj;
	}
private:
	int m_nSize;
	Type *m_pObj;
};

