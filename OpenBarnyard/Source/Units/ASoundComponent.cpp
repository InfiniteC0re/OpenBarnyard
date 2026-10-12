#include "pch.h"
#include "ASoundComponent.h"
#include "AGeneralUnitComponentManager.h"
#include "AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Sound/ASoundManager.h"
#include "FSM/AMethodPointer.h"
#include "ARandom.h"

#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005d2e20, CreateObject 005d3190, CreateObjectInPlace 005d31f0 (the constructor is inlined in both)
// $Barnyard: FUNCTION 005d3190
// $Barnyard: FUNCTION 005d31f0
TDEFINE_CLASS_INIT( ASoundComponent );

// NOTE: GetClass 005d2dc0, CreateObject 005d2e60, CreateObjectInPlace 005d2dd0 (the constructor is inlined in both)
// $Barnyard: FUNCTION 005d2e60
// $Barnyard: FUNCTION 005d2dd0
TDEFINE_CLASS( ASoundComponentData );

AFunctionTable ASoundComponent::ms_oFunctionTable( &TGetClass( ASoundComponent ), &AUnitComponent::ms_oFunctionTable );

// $Barnyard: FUNCTION 005d32a0
void ASoundComponent::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AUnitComponentFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer3Args<ASoundComponent, ASoundId, TFLOAT, TFLOAT>::Create( "PlaySound", &ASoundComponent::PlaySound ) );
}

// $Barnyard: FUNCTION 005d2e30
void ASoundComponent::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 005d2ec0
ASoundComponent::ASoundComponent()
{
	m_iCueIndex     = -1;
	m_fTimeBudget   = 0.0f;
	m_fLastPlayTime = g_oSystemManager.GetScheduler()->GetTotalTime() - 1.0f;
}

// $Barnyard: FUNCTION 005d2e40
// $Barnyard: FUNCTION 005d2f30
ASoundComponent::~ASoundComponent()
{
}

// NOTE: 005d2e50, listed in the ignore list
AFunctionTable* ASoundComponent::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// NOTE: folded with AUnitProfileComponent::GetManager (005d2300)
AUnitComponentManager* ASoundComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005d2f50
void ASoundComponent::PlaySound( ASoundId a_iSound, TFLOAT a_fChance, TFLOAT a_fMinInterval )
{
	if ( m_iCueIndex != -1 && ASoundManager::GetSingleton()->IsStreamCueActive( m_iCueIndex ) )
		return;

	TFLOAT fTotalTime = g_oSystemManager.GetScheduler()->GetTotalTime();
	TFLOAT fElapsed   = fTotalTime - m_fLastPlayTime;
	m_fTimeBudget += fElapsed;

	if ( fElapsed > a_fMinInterval && m_fTimeBudget > 0.25f )
	{
		if ( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, 1.0f / ( fElapsed * 0.1f ) ) < a_fChance )
		{
			m_fTimeBudget -= 0.25f;
			m_fLastPlayTime = fTotalTime - 0.0001f;

			m_iCueIndex = ASoundManager::GetSingleton()->PlayCueAtPosition( a_iSound, *GetOwner()->GetSteer()->GetPosition() );
		}
	}
}

// NOTE: inlined into CreateObject 005d2e60 / CreateObjectInPlace 005d2dd0
ASoundComponentData::ASoundComponentData()
{
}

// NOTE: the destructor is the shared one at 0061fa90
ASoundComponentData::~ASoundComponentData()
{
}

// $Barnyard: FUNCTION 005d3230
void ASoundComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	ASoundComponent* pComponent = new ASoundComponent();
	pComponent->m_pStaticData   = this;

	a_pUnit->AddComponent( pComponent );
}
