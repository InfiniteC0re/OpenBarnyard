#pragma once
#include "PrimeDebug.h"

#ifdef PRIMEDEBUG

class PCFunc;

// Returns TTRUE on success
typedef TBOOL ( *PCFuncCallback )( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );

//-----------------------------------------------------------------------------
// Purpose: a console command (Prime's crCFunc). Registered with a caller-owned
// object and kept in a list sorted by name (PConsole).
//-----------------------------------------------------------------------------
class PCFunc
{
public:
	friend class PConsole;

	static constexpr TINT ANY_NUM_ARGS = -1;

public:
	PCFunc()
	    : m_fnCallback( TNULL )
	    , m_iNumArgs( 0 )
	    , m_szName( TNULL )
	    , m_szDescription( TNULL )
	    , m_szUsage( TNULL )
	    , m_pUserData( TNULL )
	    , m_pNext( TNULL )
	    , m_pPrev( TNULL )
	{}

	// a_iNumArgs: the number of arguments after the name, ANY_NUM_ARGS for any. The strings must outlive the command
	TBOOL Create( const TCHAR* a_szName, const TCHAR* a_szDescription, const TCHAR* a_szUsage, PCFuncCallback a_fnCallback, TINT a_iNumArgs, void* a_pUserData );
	void  Remove();

	const TCHAR* GetName() const { return m_szName; }
	const TCHAR* GetDescription() const { return m_szDescription; }
	const TCHAR* GetUsage() const { return m_szUsage; }
	void*        GetUserData() const { return m_pUserData; }
	PCFunc*      GetNext() const { return m_pNext; }

private:
	PCFuncCallback m_fnCallback;    // 0x00
	TINT           m_iNumArgs;      // 0x04, -1: any number
	const TCHAR*   m_szName;        // 0x08
	const TCHAR*   m_szDescription; // 0x0C
	const TCHAR*   m_szUsage;       // 0x10
	void*          m_pUserData;     // 0x14
	PCFunc*        m_pNext;         // 0x18
	PCFunc*        m_pPrev;         // 0x1C
};

#endif // PRIMEDEBUG
