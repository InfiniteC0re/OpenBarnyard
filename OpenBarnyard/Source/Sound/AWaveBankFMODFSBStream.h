#pragma once
#include "AWaveBank.h"

class AWaveBankFMODFSBStream : public AWaveBank
{
public:
	AWaveBankFMODFSBStream( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strPath );

	virtual TBOOL IsReady() const OVERRIDE;
	virtual LOADRESULT Load( TBOOL a_bLocalise ) OVERRIDE;
	virtual UNLOADRESULT Unload() OVERRIDE;

};
