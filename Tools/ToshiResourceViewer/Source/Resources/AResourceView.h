#pragma once
#include <Toshi2/T2DList.h>
#include <Toshi/Core/TRefCounted.h>
#include <Toshi/File/TTRB.h>

class AResourceFilePtr;
class AResourceFile : public Toshi::TRefCounted
{
public:
	static AResourceFilePtr Create();

	TBOOL LoadFile(const char* a_szFileName)
	{
		m_sFileName = a_szFileName;
		return m_TRB.Load(a_szFileName) == Toshi::TTRB::ERROR_OK;
	}

	const Toshi::TString8& GetFileName() const
	{
		return m_sFileName;
	}

	Toshi::TTRB& GetTRB()
	{
		return m_TRB;
	}

	const Toshi::TTRB& GetTRB() const
	{
		return m_TRB;
	}

private:
	AResourceFile() = default;

private:
	Toshi::TString8 m_sFileName;
	Toshi::TTRB m_TRB;
};

class AResourceFilePtr
{
public:
	AResourceFilePtr()
	{
		m_pPtr = TNULL;
	}

	AResourceFilePtr(AResourceFile* a_pResFile)
	{
		m_pPtr = a_pResFile;
		
		if (m_pPtr)
			m_pPtr->IncRefCount();
	}

	AResourceFilePtr(const AResourceFilePtr& a_rOther)
	{
		m_pPtr = a_rOther.m_pPtr;
		
		if (m_pPtr)
			m_pPtr->IncRefCount();
	}

	~AResourceFilePtr()
	{
		DecRef();
	}

	AResourceFile* GetPointer()
	{
		return m_pPtr;
	}

	const AResourceFile* GetPointer() const
	{
		return m_pPtr;
	}

	AResourceFile* operator->()
	{
		return m_pPtr;
	}

	const AResourceFile* operator->() const
	{
		return m_pPtr;
	}

	AResourceFilePtr& operator=(AResourceFile* a_pResFile)
	{
		DecRef();
		m_pPtr = a_pResFile;
		m_pPtr->IncRefCount();
		return *this;
	}

	AResourceFilePtr& operator=(const AResourceFilePtr& a_rFilePtr)
	{
		DecRef();
		m_pPtr = a_rFilePtr.m_pPtr;
		m_pPtr->IncRefCount();
		return *this;
	}

private:
	void DecRef()
	{
		if (m_pPtr)
		{
			m_pPtr->DecRefCount();

			if (m_pPtr->GetRefCount() == 0)
				delete m_pPtr;
		}
	}

private:
	AResourceFile* m_pPtr;
};

class AResourceView : public Toshi::T2DList<AResourceView>::Node
{
public:
	AResourceView(AResourceFilePtr a_pResourceFile) :
		m_pFile(a_pResourceFile)
	{
		OnCreate();
		m_bToBeDestroyed = TFALSE;
	}
	
	virtual ~AResourceView()
	{
		OnDestroy();
	}

	virtual TBOOL SupportsSave() { return TFALSE; }
	virtual void Save() { }

	virtual void Render() { }
	virtual void OnCreate() { }
	virtual void OnDestroy() { }

	void Destroy()
	{
		m_bToBeDestroyed = TTRUE;
	}

	TBOOL IsSetToBeDestroyed() const
	{
		return m_bToBeDestroyed;
	}

	AResourceFilePtr GetFile()
	{
		return m_pFile;
	}

	const AResourceFilePtr GetFile() const
	{
		return m_pFile;
	}
	
private:
	AResourceFilePtr m_pFile;
	TBOOL m_bToBeDestroyed;
};

inline AResourceFilePtr AResourceFile::Create()
{
	return AResourceFilePtr(new AResourceFile);
}
