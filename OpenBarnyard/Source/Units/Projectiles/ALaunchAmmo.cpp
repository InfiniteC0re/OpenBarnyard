#include "pch.h"
#include "ALaunchAmmo.h"
#include "ASimpleProjectileManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00666300, CreateObject 00666340, CreateObjectInPlace 006663d0
TDEFINE_CLASS( ALaunchAmmo );

// $Barnyard: FUNCTION 00666310
ALaunchAmmo::ALaunchAmmo()
    : m_iProjectileType( ASimpleProjectileManager::TYPE_SIM_MILK )
    , m_iMaxAmmo( 5 )
    , m_fRechargeRate( 0.5f )
    , m_fAmmo( 0.0f )
    , m_fUnk14( 30.0f )
{
}

// NOTE: the deleting destructor is shared with other TObject subclasses (0057fd50)
ALaunchAmmo::~ALaunchAmmo()
{
}

// $Barnyard: FUNCTION 00666460
void ALaunchAmmo::Update( TFLOAT a_fDeltaTime )
{
	TFLOAT fAmmo = a_fDeltaTime * m_fRechargeRate + m_fAmmo;

	if ( TFLOAT( m_iMaxAmmo ) < fAmmo )
		m_fAmmo = TFLOAT( m_iMaxAmmo );
	else
		m_fAmmo = fAmmo;
}

// $Barnyard: FUNCTION 004148d0
void ALaunchAmmo::SetAmmo( TINT a_iAmmo )
{
	if ( m_iMaxAmmo < a_iAmmo )
		m_fAmmo = TFLOAT( m_iMaxAmmo );
	else
		m_fAmmo = TFLOAT( a_iAmmo );
}

// $Barnyard: FUNCTION 00668790
void ALaunchAmmo::AddAmmo()
{
	TFLOAT fAmmo = m_fAmmo + 1.0f;

	if ( TFLOAT( m_iMaxAmmo ) < fAmmo )
		m_fAmmo = TFLOAT( m_iMaxAmmo );
	else
		m_fAmmo = fAmmo;
}

// $Barnyard: FUNCTION 00668750
void ALaunchAmmo::UseAmmo()
{
	TFLOAT fAmmo = m_fAmmo - 1.0f;

	if ( fAmmo < 0.0f )
		m_fAmmo = 0.0f;
	else
		m_fAmmo = fAmmo;
}
