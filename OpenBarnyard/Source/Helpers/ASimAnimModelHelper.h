#pragma once

class ASimAnimModelHelper
{
public:
	ASimAnimModelHelper()  = default;
	~ASimAnimModelHelper() = default;

	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) = 0;
	virtual void  OnDestroy()                     = 0;
};
