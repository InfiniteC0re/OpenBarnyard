#pragma once
#include "ACollisionObjectSimple.h"

//-----------------------------------------------------------------------------
// Purpose: Temporary collision sphere that never goes into the AObjectHash, used
// to sweep a sphere against the collision objects (see the camera wall test).
// ACollisionManager::CollideSweptSphereObjects skips objects of this class.
// NOTE: TClass 0x007b4770, vtable 0x00723350.
//-----------------------------------------------------------------------------
class ACollisionObjectSimpleDummy
    : public ACollisionObjectSimple
{
public:
	TDECLARE_CLASS( ACollisionObjectSimpleDummy, ACollisionObjectSimple );

public:
	ACollisionObjectSimpleDummy();
	~ACollisionObjectSimpleDummy();

	//-----------------------------------------------------------------------------
	// ACollisionObject
	//-----------------------------------------------------------------------------
	virtual TBOOL UpdateHash( TFLOAT a_fRadius ) OVERRIDE; // +0x4c

	//-----------------------------------------------------------------------------
	// ACollisionObjectSimple
	//-----------------------------------------------------------------------------
	virtual TBOOL UpdateHash( TFLOAT a_fRadius, const Toshi::TVector3& a_rcPosition ) OVERRIDE; // +0x64
	virtual void  UpdateHash( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                   // +0x68

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Unk_0x6c() {} // +0x6c, DOES_NOTHING 005acd80
};

TSTATICASSERT( sizeof( ACollisionObjectSimpleDummy ) == 0x38 );
