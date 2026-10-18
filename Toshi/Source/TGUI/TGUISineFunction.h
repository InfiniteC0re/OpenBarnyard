#pragma once
#include "TGUIFunction.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: sine wave (period of 2) TGUIFunction.
//-----------------------------------------------------------------------------
class TGUISineFunction : public TGUIFunction
{
public:
	TDECLARE_CLASS( TGUISineFunction, TGUIFunction );

public:
	TGUISineFunction();
	TGUISineFunction( TFLOAT a_fFrequency, TFLOAT a_fPhase, TFLOAT a_fScale, TFLOAT a_fOffset );
	TGUISineFunction( const TGUISineFunction& a_rcOther );
	~TGUISineFunction();

	virtual TGUISineFunction* Clone() const override;
	virtual TFLOAT ComputeNormalised( TFLOAT a_fX ) const override;
};

TOSHI_NAMESPACE_END
