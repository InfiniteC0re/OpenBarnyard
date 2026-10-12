#include "pch.h"
#include "AGiftComponent.h"
#include "AGift.h"
#include "AUnit.h"
#include "AGeneralUnitComponentManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005d1450, CreateObject 005d15a0, CreateObjectInPlace 005d1610
TDEFINE_CLASS( AGiftComponent );

// NOTE: GetClass 005d13f0, CreateObject 005d1540, CreateObjectInPlace 005d1400
TDEFINE_CLASS( AGiftComponentData );

// NOTE: inlined into CreateObject / CreateObjectInPlace / AGiftComponentData::OnUnitCreate
AGiftComponent::AGiftComponent()
    : m_pGift( TNULL )
{
}

// $Barnyard: FUNCTION 005d1700
// $Barnyard: FUNCTION 005d1460
AGiftComponent::~AGiftComponent()
{
	if ( m_pGift )
		delete m_pGift;
}

// NOTE: same code as AUnitProfileComponent::GetManager, folded in retail
AUnitComponentManager* AGiftComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005d1500
void AGiftComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( m_pGift && m_pGift->AssignInteraction( GetOwner(), a_fDeltaTime ) == TTRUE )
	{
		if ( m_pGift )
			delete m_pGift;

		m_pGift = TNULL;
	}
}

// $Barnyard: FUNCTION 005d14c0
TBOOL AGiftComponent::GiveGift( AGift* a_pGift )
{
	if ( m_pGift || !a_pGift )
		return TFALSE;

	m_pGift = a_pGift;
	a_pGift->ResetTime();

	return TTRUE;
}

// $Barnyard: FUNCTION 005d14f0
AGift* AGiftComponent::TakeGift()
{
	AGift* pGift = m_pGift;
	m_pGift      = TNULL;

	return pGift;
}

// NOTE: inlined into CreateObject / CreateObjectInPlace
AGiftComponentData::AGiftComponentData()
{
}

// NOTE: the destructor is shared with other static data (0061fa90)
AGiftComponentData::~AGiftComponentData()
{
}

// $Barnyard: FUNCTION 005d1680
void AGiftComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AGiftComponent* pComponent = new AGiftComponent();
	pComponent->m_pData        = this;

	a_pUnit->AddComponent( pComponent );
}
