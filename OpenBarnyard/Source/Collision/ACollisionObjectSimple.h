#pragma once
#include "ACollisionObject.h"

class ACollisionObjectSimple
    : public ACollisionObject
{
public:
	TDECLARE_CLASS( ACollisionObjectSimple, ACollisionObject );

public:
	ACollisionObjectSimple();
	~ACollisionObjectSimple();

	//-----------------------------------------------------------------------------
	// ACollisionObject
	//-----------------------------------------------------------------------------
	virtual void VTable0x1C( void* ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Checks if the position or the radius has changed and updates hash if needed
	virtual TBOOL UpdateHash( TFLOAT a_fRadius, const Toshi::TVector3& a_rcPosition ); // VTable: 0x64

	// Checks if the position has changed and updates hash if needed
	virtual void UpdateHash( const Toshi::TVector3& a_rcPosition ); // VTable: 0x68

private:
	Toshi::TVector3 m_vPosition;
	TFLOAT          m_fRadius;
	TFLOAT          m_fHeightScale;
	TINT            field4_0x34;
};
