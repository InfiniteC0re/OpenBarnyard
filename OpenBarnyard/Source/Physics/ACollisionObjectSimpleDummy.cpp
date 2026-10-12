#include "pch.h"
#include "ACollisionObjectSimpleDummy.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0061bd70, CreateObject 0061c8c0, CreateObjectInPlace 0061c950
TDEFINE_CLASS( ACollisionObjectSimpleDummy );

// NOTE: inlined in retail (the TClass create function 0061c8c0 and SweepSphere in AGTAStyleCameraHelper.cpp)
ACollisionObjectSimpleDummy::ACollisionObjectSimpleDummy()
{
}

// $Barnyard: FUNCTION 0052de90
// $Barnyard: FUNCTION 0052deb0
// $Barnyard: FUNCTION 0052dff0
ACollisionObjectSimpleDummy::~ACollisionObjectSimpleDummy()
{
}

// $Barnyard: FUNCTION 0061c0e0
TBOOL ACollisionObjectSimpleDummy::UpdateHash( TFLOAT a_fRadius )
{
	m_fRadius = a_fRadius;
	return TTRUE;
}

// $Barnyard: FUNCTION 0061c110
TBOOL ACollisionObjectSimpleDummy::UpdateHash( TFLOAT a_fRadius, const TVector3& a_rcPosition )
{
	m_fRadius   = a_fRadius;
	m_vPosition = a_rcPosition;
	return TTRUE;
}

// $Barnyard: FUNCTION 0061c0f0
void ACollisionObjectSimpleDummy::UpdateHash( const TVector3& a_rcPosition )
{
	m_vPosition = a_rcPosition;
}
