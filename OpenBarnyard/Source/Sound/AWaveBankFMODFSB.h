#pragma once
#include "AWaveBank.h"

class AWaveBankFMODFSB : public AWaveBank
{
public:
    AWaveBankFMODFSB( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strPath );

    virtual TBOOL        IsReady() const OVERRIDE;
    virtual LOADRESULT   Load( LOADFLAGS a_uiFlags, TINT a_iBufferSize ) OVERRIDE;
    virtual UNLOADRESULT Unload() OVERRIDE;
};
