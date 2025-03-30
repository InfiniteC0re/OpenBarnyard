#pragma once
#include "Toshi/TString8.h"

TOSHI_NAMESPACE_START

class TOSHI_API TSystemDLLWin
{
public:
	TSystemDLLWin();
	TSystemDLLWin( const TSystemDLLWin& other );

	virtual ~TSystemDLLWin();

	TBOOL Load( const TString8& a_sFileName );
	void  Unload();

	void* GetAddress( const TString8& a_sSymbolName );

	const TString8& GetFileName() const { return m_sFileName; }
	void*           GetDLL() const { return m_pDLL; }

	TSystemDLLWin& operator=( const TSystemDLLWin& other );

private:
	TString8 m_sFileName;
	void*    m_pDLL;
};

TOSHI_NAMESPACE_END
