#pragma once
#include "TGUIFunction.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: sawtooth wave (period of 1, from -1 to 1) TGUIFunction.
//-----------------------------------------------------------------------------
class TGUISawtoothFunction : public TGUIFunction
{
public:
	TDECLARE_CLASS( TGUISawtoothFunction, TGUIFunction );

public:
	TGUISawtoothFunction();
	TGUISawtoothFunction( TFLOAT a_fFrequency, TFLOAT a_fPhase, TFLOAT a_fScale, TFLOAT a_fOffset );
	TGUISawtoothFunction( const TGUISawtoothFunction& a_rcOther );
	~TGUISawtoothFunction();

	virtual TGUISawtoothFunction* Clone() const override;
	virtual TFLOAT ComputeNormalised( TFLOAT a_fX ) const override;
};

TOSHI_NAMESPACE_END
