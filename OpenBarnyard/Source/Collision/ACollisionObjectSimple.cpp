#include "pch.h"
#include "ACollisionObjectSimple.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// TODO: make it runtime constructable when all virtual methods are implemented
TDEFINE_CLASS_NORUNTIME( ACollisionObjectSimple );

// $Barnyard: FUNCTION 0061c5b0
ACollisionObjectSimple::ACollisionObjectSimple()
    : m_fHeightScale( 0.0f )
    , field4_0x34( 0 )
{
	m_eType = Type_Simple;
}

ACollisionObjectSimple::~ACollisionObjectSimple()
{
	TASSERT( TFALSE && "Not implemented" );
}

void ACollisionObjectSimple::VTable0x1C( void* )
{
}

// $Barnyard: FUNCTION 0061c010
TBOOL ACollisionObjectSimple::UpdateHash( TFLOAT a_fRadius, const Toshi::TVector3& a_rcPosition )
{
	TBOOL bRadiusChanged = a_fRadius != m_fRadius;

	if ( bRadiusChanged )
		m_fRadius = a_fRadius;

	if ( !m_vPosition.Equals( a_rcPosition ) )
	{
		m_vPosition = a_rcPosition;
		ACollisionObject::CreateHash();
	}
	else if ( bRadiusChanged )
	{
		ACollisionObject::CreateHash();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0061bfd0
void ACollisionObjectSimple::UpdateHash( const Toshi::TVector3& a_rcPosition )
{
	if ( !m_vPosition.Equals( a_rcPosition ) )
	{
		m_vPosition = a_rcPosition;
		ACollisionObject::CreateHash();
	}
}
