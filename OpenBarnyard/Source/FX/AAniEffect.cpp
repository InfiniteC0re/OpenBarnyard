#include "pch.h"
#include "AAniEffect.h"
#include "AAniEffectManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0062f7d0, CreateObject 0062f820, CreateObjectInPlace 0062f8a0
TDEFINE_CLASS( AAniEffect );

// $Barnyard: FUNCTION 0062f7e0
AAniEffect::AAniEffect()
    : m_pNextEffect( TNULL )
    , m_eFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 0062f9b0
// $Barnyard: FUNCTION 0062f910
AAniEffect::~AAniEffect()
{
	if ( AAniEffectManager::GetSingleton() )
		AAniEffectManager::GetSingleton()->UnregisterEffect( this );
}

// $Barnyard: FUNCTION 0062f800
void AAniEffect::SetActive( TBOOL a_bActive )
{
	if ( a_bActive )
		m_eFlags |= FLAGS_ACTIVE;
	else
		m_eFlags &= ~FLAGS_ACTIVE;
}

// $Barnyard: FUNCTION 0062f970
void AAniEffect::SetNextEffect( AAniEffect* a_pNextEffect )
{
	m_eFlags |= FLAGS_ACTIVE;
	m_pNextEffect = a_pNextEffect;

	if ( a_pNextEffect )
		a_pNextEffect->m_eFlags &= ~FLAGS_ACTIVE;
}

// $Barnyard: FUNCTION 0062f990
void AAniEffect::Finish()
{
	if ( m_pNextEffect )
		m_pNextEffect->m_eFlags |= FLAGS_ACTIVE;

	AAniEffectManager::GetSingleton()->KillEffect( this );
}
