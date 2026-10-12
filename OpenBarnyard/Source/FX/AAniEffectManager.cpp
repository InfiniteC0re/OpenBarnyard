#include "pch.h"
#include "AAniEffectManager.h"
#include "Memory/AMemory.h"
#include "Units/AUnitManager.h"
#include "Units/AVisibleUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0062fd70
AAniEffectManager::AAniEffectManager()
    : m_oEffectPool( AMemory::GetAllocator( AMemory::POOL_Misc ), MAX_NUM_EFFECTS )
{
}

// $Barnyard: FUNCTION 006300b0
// $Barnyard: FUNCTION 00630040
AAniEffectManager::~AAniEffectManager()
{
	DeleteAllEffects();
}

// $Barnyard: FUNCTION 0062fd00
TINT AAniEffectManager::RemoveUnitEffects( AUnit* a_pUnit )
{
	TINT iNumRemoved = 0;

	for ( TINT i = m_vecEffects.Size() - 1; i >= 0; i-- )
	{
		AAniEffect* pEffect = m_vecEffects[ i ];

		if ( pEffect->GetUnit() == a_pUnit )
		{
			iNumRemoved += RemoveEffect( pEffect );

			// Several effects could have been removed
			if ( i >= m_vecEffects.Size() )
				i = m_vecEffects.Size() - 1;
		}
	}

	return iNumRemoved;
}

// $Barnyard: FUNCTION 0062fc80
TINT AAniEffectManager::RemoveEffect( AAniEffect* a_pEffect )
{
	TINT iNumRemoved = 0;

	// Remove the effects chained after this one first
	if ( a_pEffect->GetNextEffect() )
		iNumRemoved = RemoveEffect( a_pEffect->GetNextEffect() );

	// Unlink from the effect it's chained after
	AAniEffect* pPrevEffect = FindPreviousEffect( a_pEffect );

	if ( pPrevEffect )
		pPrevEffect->SetNextEffect( TNULL );

	UnregisterEffect( a_pEffect );
	KillEffect( a_pEffect );

	return iNumRemoved + 1;
}

// $Barnyard: FUNCTION 0062fa90
void AAniEffectManager::KillEffect( AAniEffect* a_pEffect )
{
	if ( a_pEffect && !a_pEffect->IsDead() )
	{
		a_pEffect->m_eFlags |= AAniEffect::FLAGS_DEAD;
		a_pEffect->SetActive( TFALSE );
		m_vecDeadEffects.PushBack( a_pEffect );
	}
}

// $Barnyard: FUNCTION 0062fc20
void AAniEffectManager::UnregisterEffect( AAniEffect* a_pEffect )
{
	m_vecEffects.FindAndErase( a_pEffect );
}

// $Barnyard: FUNCTION 0062fa50
AAniEffect* AAniEffectManager::FindPreviousEffect( AAniEffect* a_pEffect )
{
	for ( TINT i = m_vecEffects.Size() - 1; i >= 0; i-- )
	{
		AAniEffect* pEffect = m_vecEffects[ i ];

		if ( pEffect->GetNextEffect() == a_pEffect )
			return pEffect;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0062fde0
void AAniEffectManager::DeleteDeadEffects()
{
	for ( TINT i = m_vecDeadEffects.Size() - 1; i >= 0; i-- )
		m_oEffectPool.DeleteObject( m_vecDeadEffects[ i ] );

	m_vecDeadEffects.Clear();
}

// $Barnyard: FUNCTION 0062ff90
void AAniEffectManager::DeleteAllEffects()
{
	DeleteDeadEffects();

	// NOTE: the destructor of each effect unregisters it from m_vecEffects
	for ( TINT i = m_vecEffects.Size() - 1; i >= 0; i-- )
		m_oEffectPool.DeleteObject( m_vecEffects[ i ] );

	m_vecEffects.Clear();
}

// $Barnyard: FUNCTION 00630000
void AAniEffectManager::Update( TFLOAT a_fDeltaTime )
{
	DeleteDeadEffects();

	for ( TINT i = m_vecEffects.Size() - 1; i >= 0; i-- )
	{
		AAniEffect* pEffect = m_vecEffects[ i ];

		if ( pEffect->IsActive() )
			pEffect->Update( a_fDeltaTime );

		// Several effects could have been removed
		if ( i >= m_vecEffects.Size() )
			i = m_vecEffects.Size() - 1;
	}

	DeleteDeadEffects();
}

// $Barnyard: FUNCTION 0062fe60
TINT AAniEffectManager::DisposeInvisibleUnits( TINT a_iNumEffects )
{
	DeleteDeadEffects();

	TINT iNumRemoved = 0;

	for ( TINT i = m_vecEffects.Size() - 1; i >= 0 && iNumRemoved < a_iNumEffects; i-- )
	{
		AAniEffect* pEffect = m_vecEffects[ i ];

		if ( pEffect->IsUnitDisposable() )
		{
			AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, pEffect->GetUnit() );

			if ( !pUnit->IsInCameraFrustum() )
			{
				iNumRemoved += RemoveUnitEffects( pUnit );
				AUnitManager::GetSingleton()->DestroyUnit( pUnit );

				// Several effects could have been removed
				if ( i >= m_vecEffects.Size() )
					i = m_vecEffects.Size() - 1;
			}
		}
	}

	DeleteDeadEffects();
	return iNumRemoved;
}

// $Barnyard: FUNCTION 0062fee0
TINT AAniEffectManager::DisposeUnits( TINT a_iNumEffects )
{
	DeleteDeadEffects();

	TINT iNumRemoved = DisposeInvisibleUnits( a_iNumEffects );

	for ( TINT i = m_vecEffects.Size() - 1; i >= 0 && iNumRemoved < a_iNumEffects; i-- )
	{
		AAniEffect* pEffect = m_vecEffects[ i ];

		if ( pEffect->IsUnitDisposable() )
		{
			AUnit* pUnit = pEffect->GetUnit();
			iNumRemoved += RemoveUnitEffects( pUnit );

			if ( pUnit )
				AUnitManager::GetSingleton()->DestroyUnit( pUnit );

			// Several effects could have been removed
			if ( i >= m_vecEffects.Size() )
				i = m_vecEffects.Size() - 1;

			iNumRemoved++;
		}
	}

	DeleteDeadEffects();
	return iNumRemoved;
}

// $Barnyard: FUNCTION 0062ff60
TBOOL AAniEffectManager::ReserveEffects( TINT a_iNumEffects )
{
	return MAX_NUM_EFFECTS - m_vecDeadEffects.Size() - m_vecEffects.Size() > a_iNumEffects || DisposeUnits( a_iNumEffects ) >= a_iNumEffects;
}
