#pragma once
#include "Toshi/TString8.h"

TOSHI_NAMESPACE_START

class TOSHI_API TSystemDLLWASM
{
public:
	TSystemDLLWASM();
	TSystemDLLWASM( const TSystemDLLWASM& other );

	virtual ~TSystemDLLWASM();

	TBOOL Load( const TString8& a_sFileName );
	void  Unload();

	void* GetAddress( const TString8& a_sSymbolName );

	const TString8& GetFileName() const { return m_sFileName; }
	void*           GetDLL() const { return m_pDLL; }

	TSystemDLLWASM& operator=( const TSystemDLLWASM& other );

private:
	TString8 m_sFileName;
	void*    m_pDLL;
};

TOSHI_NAMESPACE_END
