#include "pch.h"
#include "AHudManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS( AHudManager );

AHudManager::AHudManager()
{
}

// $Barnyard: FUNCTION 0044f160
// $Barnyard: FUNCTION 0044eff0
AHudManager::~AHudManager()
{
}

// $Barnyard: FUNCTION 0044f350
TBOOL AHudManager::Reset()
{
	T2_FOREACH( m_Elements, it )
	{
		it->Reset();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0044f4b0
TBOOL AHudManager::OnCreate()
{
	// TODO: The original creates AInteractionHudElement, ATerrainSectionHudElement, and AMenuNavHudElement here.
	TIMPLEMENT();

	return TTRUE;
}

// $Barnyard: FUNCTION 0044f270
TBOOL AHudManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	TUINT32 uiHUDMask = GetCurrentHUDMask();

	T2_FOREACH( m_Elements, it )
	{
		if ( it->m_bForceHide || ( uiHUDMask & it->m_eMask ) == 0 )
		{
			if ( it->m_bCurrentlyVisible )
			{
				it->Hide();
			}
		}
		else
		{
			if ( !it->m_bCurrentlyVisible )
			{
				it->Show();
			}

			if ( it->m_bCurrentlyVisible )
			{
				it->Update( a_fDeltaTime );
			}
		}
	}

	// TODO: The original also mirrors HUD mask bit 0x80 into a field of AAnimalPopulationManager
	TTODO( "AAnimalPopulationManager" );

	return TTRUE;
}

// $Barnyard: FUNCTION 0044f010
void AHudManager::OnDestroy()
{
	while ( !m_Elements.IsEmpty() )
	{
		AHudElement* pElement = m_Elements.Begin();
		pElement->OnDetach();
		m_Elements.Erase( pElement );

		if ( pElement->m_bOwnedByManager )
		{
			delete pElement;
		}
	}
}

// $Barnyard: FUNCTION 0044f180
void AHudManager::OnActivate()
{
	UpdateElementsVisibility( TTRUE );
}

// $Barnyard: FUNCTION 0044f200
void AHudManager::OnDeactivate()
{
	TUINT32 uiHUDMask = GetCurrentHUDMask();

	T2_FOREACH( m_Elements, it )
	{
		if ( ( uiHUDMask & it->m_eMask ) != 0 && it->m_bCurrentlyVisible )
		{
			it->Hide();
		}
	}
}

// $Barnyard: FUNCTION 0044f3a0
TBOOL AHudManager::AddElement( AHudElement* a_pElement, TBOOL a_bOwnedByManager, TBOOL a_bInsertTail, TBOOL a_bShow )
{
	TVALIDPTR( a_pElement );

	if ( !a_pElement->OnAttach() )
	{
		return TFALSE;
	}

	if ( a_bShow )
	{
		a_pElement->Show();
	}

	a_pElement->m_bOwnedByManager = a_bOwnedByManager;

	if ( a_bInsertTail )
	{
		m_Elements.PushBack( a_pElement );
	}
	else
	{
		m_Elements.PushFront( a_pElement );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0044f440
AHudElement* AHudManager::FindElement( const Toshi::TClass* a_pClass )
{
	T2_FOREACH( m_Elements, it )
	{
		if ( it->GetClass()->IsA( a_pClass ) )
		{
			return it;
		}
	}

	return TNULL;
}

TUINT32 AHudManager::GetCurrentHUDMask() const
{
	return AGameStateController::GetSingleton()->GetCurrentState()->GetHUDMask();
}

void AHudManager::UpdateElementsVisibility( TBOOL a_bForceShow )
{
	TUINT32 uiHUDMask = GetCurrentHUDMask();

	T2_FOREACH( m_Elements, it )
	{
		if ( ( uiHUDMask & it->m_eMask ) != 0 && !it->m_bForceHide && !it->m_bCurrentlyVisible )
		{
			it->Show();
		}
	}
}
