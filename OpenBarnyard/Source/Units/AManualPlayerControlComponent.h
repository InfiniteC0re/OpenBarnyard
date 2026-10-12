#pragma once
#include "ASimplePlayerControlComponent.h"

//-----------------------------------------------------------------------------
// Control component of the units driven by code instead of a controller, such
// as the Chasing Chicks AI rivals (AChasingChicksAI sets the movement input).
// The movement is relative to the world: the "camera" always faces +Z.
// TClass registered at 0x0070b594, vtable 00728858, size 0x3c.
//-----------------------------------------------------------------------------
class AManualPlayerControlComponent
    : public ASimplePlayerControlComponent
{
public:
	TDECLARE_CLASS( AManualPlayerControlComponent, ASimplePlayerControlComponent );

public:
	AManualPlayerControlComponent();
	~AManualPlayerControlComponent();

	//-----------------------------------------------------------------------------
	// ASimplePlayerControlComponent
	//-----------------------------------------------------------------------------
	virtual void GetCameraForward( Toshi::TVector4& a_rOutForward ) OVERRIDE; // +0x48
};
