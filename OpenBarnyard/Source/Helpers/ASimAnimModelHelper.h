#pragma once

class AModelInstance;

class ASimAnimModelHelper
{
public:
	ASimAnimModelHelper()  = default;
	~ASimAnimModelHelper() = default;

	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) = 0;
	virtual void  OnDestroy()                     = 0;

	// Returns TTRUE if a_pModel is closer to the camera than the far plane (with 4 units^2 of slack).
	// NOTE: inlined into the update functions of the farm objects in retail
	static TBOOL IsInAnimationRange( AModelInstance* a_pModel );
};
