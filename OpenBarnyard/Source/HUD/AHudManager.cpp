#include "pch.h"
#include "AHudManager.h"
#include "AInteractionHudElement.h"
#include "AMenuNavHudElement.h"
#include "ATerrainSectionHudElement.h"
#include "Units/AAnimalPopulationManager.h"
#include "GUI/AGUI2ModelRenderer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// NOTE: GetClass 0044efe0, CreateObject 0044f080, CreateObjectInPlace 0044f100 (the constructor is inlined in both)
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
	// NOTE: the original inlines AddElement( pElement, TTRUE, TTRUE, TFALSE )
	AddElement( new AInteractionHudElement(), TTRUE, TTRUE, TFALSE );
	AddElement( new ATerrainSectionHudElement(), TTRUE, TTRUE, TFALSE );

	AMenuNavHudElement* pMenuNav  = new AMenuNavHudElement();
	pMenuNav->m_bCurrentlyVisible = TFALSE;
	AddElement( pMenuNav, TTRUE, TTRUE, TFALSE );

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

	// The quest "!" on the HUD is only rendered while the HUD shows it
	if ( AAnimalPopulationManager::GetSingleton() && AAnimalPopulationManager::GetSingleton()->GetExclamationView() )
		AAnimalPopulationManager::GetSingleton()->GetExclamationView()->SetRender( HASANYFLAG( uiHUDMask, AHudElement::MASK_INTERACTION ) );

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
