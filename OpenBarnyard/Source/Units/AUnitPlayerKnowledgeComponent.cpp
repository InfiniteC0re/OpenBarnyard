#include "pch.h"
#include "AUnitPlayerKnowledgeComponent.h"
#include "AUnit.h"
#include "AGeneralUnitComponentManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005d2b60, CreateObject 005d2c40, CreateObjectInPlace 005d2cc0
TDEFINE_CLASS( AUnitPlayerKnowledgeComponent );

// NOTE: GetClass 005d2b00, CreateObject 005d2b80, CreateObjectInPlace 005d2b10
TDEFINE_CLASS( AUnitPlayerKnowledgeComponentData );

// NOTE: inlined into CreateObject / CreateObjectInPlace / AUnitPlayerKnowledgeComponentData::OnUnitCreate
AUnitPlayerKnowledgeComponent::AUnitPlayerKnowledgeComponent()
    : m_uiKnowledge( 0 )
{
}

// $Barnyard: FUNCTION 005d2b70
// $Barnyard: FUNCTION 005d2be0
AUnitPlayerKnowledgeComponent::~AUnitPlayerKnowledgeComponent()
{
}

// NOTE: same code as AUnitProfileComponent::GetManager, folded in retail
AUnitComponentManager* AUnitPlayerKnowledgeComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005d2c00
void AUnitPlayerKnowledgeComponent::SetKnowledge( TINT a_iKnowledge )
{
	m_uiKnowledge |= 1 << a_iKnowledge;
}

// $Barnyard: FUNCTION 005d2c20
TBOOL AUnitPlayerKnowledgeComponent::HasKnowledge( TINT a_iKnowledge )
{
	return ( m_uiKnowledge & ( 1 << a_iKnowledge ) ) != 0;
}

// NOTE: inlined into CreateObject / CreateObjectInPlace
AUnitPlayerKnowledgeComponentData::AUnitPlayerKnowledgeComponentData()
{
}

// NOTE: the destructor is shared with other static data (0061fa90)
AUnitPlayerKnowledgeComponentData::~AUnitPlayerKnowledgeComponentData()
{
}

// $Barnyard: FUNCTION 005d2d30
void AUnitPlayerKnowledgeComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AUnitPlayerKnowledgeComponent* pComponent = new AUnitPlayerKnowledgeComponent();
	pComponent->m_pData                       = this;

	a_pUnit->AddComponent( pComponent );
}
