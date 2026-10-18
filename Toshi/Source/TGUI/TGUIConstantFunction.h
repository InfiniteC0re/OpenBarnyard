#pragma once
#include "TGUIFunction.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: constant (0) TGUIFunction.
//-----------------------------------------------------------------------------
class TGUIConstantFunction : public TGUIFunction
{
public:
	TDECLARE_CLASS( TGUIConstantFunction, TGUIFunction );

public:
	TGUIConstantFunction();
	TGUIConstantFunction( TFLOAT a_fFrequency, TFLOAT a_fPhase, TFLOAT a_fScale, TFLOAT a_fOffset );
	TGUIConstantFunction( const TGUIConstantFunction& a_rcOther );
	~TGUIConstantFunction();

	virtual TGUIConstantFunction* Clone() const override;
	virtual TFLOAT ComputeNormalised( TFLOAT a_fX ) const override;
};

TOSHI_NAMESPACE_END
