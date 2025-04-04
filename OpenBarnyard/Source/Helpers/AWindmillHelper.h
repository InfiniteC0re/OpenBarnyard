#pragma once
#include "ASimAnimModelHelper.h"
#include "ABarnSign.h"

class AWindmillHelper
    : public ASimAnimModelHelper
{
public:
	AWindmillHelper();
	virtual ~AWindmillHelper();

	//-----------------------------------------------------------------------------
	// ASimAnimModelHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	void CreateBarnSign();

private:
	// ...
	// ...
	TINT      m_iNumBarnSigns;
	ABarnSign m_aBarnSigns[ 1 ];
	// ...
};
