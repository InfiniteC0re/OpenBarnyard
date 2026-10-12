#include "pch.h"
#include "ADieAniEffect.h"
#include "Units/AUnit.h"
#include "Units/AUnitManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006300d0, CreateObject 006301d0, CreateObjectInPlace 00630240 (the constructor is inlined in both)
TDEFINE_CLASS( ADieAniEffect );

ADieAniEffect::ADieAniEffect()
{
}

// $Barnyard: FUNCTION 006302b0
// $Barnyard: FUNCTION 006300e0
ADieAniEffect::~ADieAniEffect()
{
	if ( m_oUnit && m_oUnit->GetUnit() )
		AUnitManager::GetSingleton()->SetUnitPoolUsed( m_oUnit->GetUnit(), TFALSE );
}

AUnit* ADieAniEffect::GetUnit()
{
	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 00630180
TBOOL ADieAniEffect::Update( TFLOAT a_fDeltaTime )
{
	if ( m_oUnit->GetUnit() )
	{
		AUnitManager::GetSingleton()->DestroyUnit( m_oUnit ? m_oUnit->GetUnit() : TNULL );
		m_oUnit = TNULL;
	}

	Finish();
	return TTRUE;
}

// $Barnyard: FUNCTION 006302d0
void ADieAniEffect::SetUnit( AUnit* a_pUnit )
{
	m_oUnit = a_pUnit->GetSelfProxy();

	// Keep the unit out of its pool until it's destroyed
	AUnitManager::GetSingleton()->SetUnitPoolUsed( m_oUnit ? m_oUnit->GetUnit() : TNULL, TTRUE );
}
