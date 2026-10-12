#include "pch.h"
#include "AStationarySteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00570220, CreateObject 005704e0, CreateObjectInPlace 00570550
TDEFINE_CLASS( AStationarySteer );

AStationarySteer::AStationarySteer()
{
	// NOTE: inlined in retail, m_vPosition and m_vDirection are left uninitialised until Create/CreateFrom
}

// $Barnyard: FUNCTION 005705a0
// $Barnyard: FUNCTION 00570380
AStationarySteer::~AStationarySteer()
{
}

// $Barnyard: FUNCTION 005703f0
TBOOL AStationarySteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ACommonSteer::CreateFrom( a_pOther, a_iLOD );

	a_pOther->CopyPosition( m_vPosition );
	a_pOther->CopyDirection( m_vDirection );

	return TTRUE;
}

// $Barnyard: FUNCTION 00570390
TBOOL AStationarySteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ACommonSteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );

	m_vPosition  = a_rcPosition;
	m_vDirection = a_rcDirection;

	return TTRUE;
}

// $Barnyard: FUNCTION 00570460
TBOOL AStationarySteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;
	return TTRUE;
}

// $Barnyard: FUNCTION 00570430
TBOOL AStationarySteer::SetPosition3( const TVector3& a_rcPosition )
{
	m_vPosition.Set( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	return TTRUE;
}

// $Barnyard: FUNCTION 00570490
void AStationarySteer::SetDirection( const TVector4& a_rcDirection )
{
	m_vDirection = a_rcDirection;
}

// $Barnyard: FUNCTION 00570250
void AStationarySteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vPosition;
}

// $Barnyard: FUNCTION 00570230
void AStationarySteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vPosition.AsVector3();
}

// $Barnyard: FUNCTION 00570270
void AStationarySteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_vDirection.AsVector3();
}

// $Barnyard: FUNCTION 005702b0
void AStationarySteer::CopyVelocity( TVector4& a_rOutVelocity )
{
	a_rOutVelocity = TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 00570310
TFLOAT AStationarySteer::GetSpeedFromModifier( TFLOAT a_fModifier )
{
	return 0.0f;
}

// $Barnyard: FUNCTION 00570320
TFLOAT AStationarySteer::GetGroundY()
{
	return m_vPosition.y;
}

// $Barnyard: FUNCTION 00570330
TVector4 AStationarySteer::GetGroundPosition()
{
	return m_vPosition;
}

// $Barnyard: FUNCTION 00570360
TVector4& AStationarySteer::PredictPosition( TINT a_iUnk, TVector4& a_rOut )
{
	a_rOut = m_vPosition;
	return a_rOut;
}

// $Barnyard: FUNCTION 005704b0
TBOOL AStationarySteer::HaltFacingDesired( const TVector3& a_rcFacing )
{
	// NOTE: retail leaves w of the temporary uninitialised
	TVector4 vecFacing;
	vecFacing.x = a_rcFacing.x;
	vecFacing.y = a_rcFacing.y;
	vecFacing.z = a_rcFacing.z;

	SetDirection( vecFacing );
	return TTRUE;
}

// $Barnyard: FUNCTION 005702e0
void AStationarySteer::Unk_0x198( TVector3& a_rOutVector )
{
	a_rOutVector = TVector3::VEC_ZERO;
}

// $Barnyard: FUNCTION 00570290
void AStationarySteer::SetPosition3NoReturn( const TVector3& a_rcPosition )
{
	m_vPosition.Set( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
}
