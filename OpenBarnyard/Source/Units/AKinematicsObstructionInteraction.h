#pragma once
#include "AKinematicsObstruction.h"

//-----------------------------------------------------------------------------
// Purpose: A circle obstruction around the units of an interaction, created by
// AKinematicsObstructionActivityMessage::CreateObstruction. Vtable 0x00727564.
//-----------------------------------------------------------------------------
class AKinematicsObstructionInteraction : public AKinematicsObstruction
{
public:
	TDECLARE_CLASS( AKinematicsObstructionInteraction, AKinematicsObstruction );

public:
	AKinematicsObstructionInteraction();
	~AKinematicsObstructionInteraction();

	//-----------------------------------------------------------------------------
	// AKinematicsObstruction
	//-----------------------------------------------------------------------------
	// $Barnyard: FUNCTION 00573660
	virtual const Toshi::TVector4* GetPosition() OVERRIDE { return &m_vPosition; }            // +0x10
	virtual Toshi::TVector4        GetDirection() OVERRIDE;                                   // +0x14
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                    // +0x18, 00573670 (same as GetDirection)
	virtual TFLOAT                 GetRadius() OVERRIDE { return m_fRadius; }                 // +0x20, 0058ac50 (shared getter of a float at +0x14)
	virtual TYPE                   GetObstructionType() OVERRIDE { return TYPE_INTERACTION; } // +0x30, 00512540 (shared, returns 4)

	void SetPosition( const Toshi::TVector4& a_rcPosition ) { m_vPosition = a_rcPosition; }
	void SetRadius( TFLOAT a_fRadius ) { m_fRadius = a_fRadius; }

private:
	Toshi::TVector4 m_vPosition; // 0x04, the middle of the units
	TFLOAT          m_fRadius;   // 0x14
};

TSTATICASSERT( sizeof( AKinematicsObstructionInteraction ) == 0x18 );
