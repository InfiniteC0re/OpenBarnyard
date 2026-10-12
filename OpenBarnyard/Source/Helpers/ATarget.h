#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: Casts a ray from the camera through the mouse cursor when asked to.
// NOTE: nothing in the retail binary reads the singleton or sets m_bUpdateRay,
// so the ray is never updated.
// Singleton at 0x00781818, vtable 0x0071a25c.
//-----------------------------------------------------------------------------
class ATarget
    : public Toshi::TTask
    , public Toshi::TSingleton<ATarget>
{
public:
	TDECLARE_CLASS( ATarget, Toshi::TTask );

public:
	ATarget();
	~ATarget();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	// Updates the ray from the current camera through the mouse cursor
	void UpdateRay();

private:
	Toshi::TVector4 m_vRayOrigin;    // 0x20
	Toshi::TVector4 m_vRayDirection; // 0x30
	Toshi::TVector4 m_vUnk40;        // 0x40
	TBOOL           m_bUpdateRay;    // 0x50
	TINT            m_iRayValid;     // 0x54
};

TSTATICASSERT( sizeof( ATarget ) == 0x58 );
