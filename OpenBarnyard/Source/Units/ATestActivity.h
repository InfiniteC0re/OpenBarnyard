#pragma once
#include "AInteractionActivity.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Debug activity: runs for "time" seconds, moving a test position along x.
//-----------------------------------------------------------------------------
class ATestActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ATestActivity, AInteractionActivity );

public:
	ATestActivity();
	~ATestActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void   GetDataSize( TINT* a_pSize ) OVERRIDE;                // +0x10
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	TFLOAT          m_fTime;       // 0x08, "time"
	Toshi::TString8 m_strPrint;    // 0x0c, "print"
	TINT            m_iDataSize;   // 0x18, grows by a TVector3 on every Create
	TINT            m_iDataOffset; // 0x1c, offset of the TVector3 of the activity in the scratch memory
};

TSTATICASSERT( sizeof( ATestActivity ) == 0x20 );
