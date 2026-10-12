#include "pch.h"
#include "ANPCMilkLauncherComponent.h"
#include "ANPCMilkLauncherComponentData.h"
#include "AGeneralUnitComponentManager.h"
#include "AUnit.h"
#include "FSM/AMethodPointer.h"
#include "Projectiles/AAimedSquirtProjectileLauncher.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 006699e0, CreateObjectInPlace 00669a60
// NOTE: GetClass 006697a0, CreateObject 006699e0, CreateObjectInPlace 00669a60
TDEFINE_CLASS_INIT( ANPCMilkLauncherComponent );

// 0x007cd248
AFunctionTable ANPCMilkLauncherComponent::ms_oFunctionTable( &TGetClass( ANPCMilkLauncherComponent ), &AUnitComponent::ms_oFunctionTable );

// $Barnyard: FUNCTION 00669cd0
void ANPCMilkLauncherComponent::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TBOOL, ANPCMilkLauncherComponent, AUnit*, TFLOAT>::Create( "FireAtTarget", &ANPCMilkLauncherComponent::FireAtTarget ) );
}

// $Barnyard: FUNCTION 006697b0
void ANPCMilkLauncherComponent::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// NOTE: inlined into CreateObject 006699e0 / CreateObjectInPlace 00669a60
ANPCMilkLauncherComponent::ANPCMilkLauncherComponent()
    : m_pLauncher( TNULL )
    , m_bFiring( TFALSE )
    , m_fFireTimer( 0.0f )
{
}

// $Barnyard: FUNCTION 00669b60
// $Barnyard: FUNCTION 006697d0
ANPCMilkLauncherComponent::~ANPCMilkLauncherComponent()
{
	if ( m_pLauncher )
		delete m_pLauncher;
}

// $Barnyard: FUNCTION 006697c0
AFunctionTable* ANPCMilkLauncherComponent::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// NOTE: folded getter of AGeneralUnitComponentManager::ms_pSingleton (005d2300)
AUnitComponentManager* ANPCMilkLauncherComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 00669940
void ANPCMilkLauncherComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	m_pLauncher->Update( a_fDeltaTime );

	if ( m_bFiring )
	{
		m_fFireTimer -= a_fDeltaTime;

		if ( m_fFireTimer < 0.0f )
		{
			m_bFiring = TFALSE;
			m_pLauncher->Deactivate();
		}
	}
}

// $Barnyard: FUNCTION 00669830
TBOOL ANPCMilkLauncherComponent::Create( AUnit* a_pOwner, ANPCMilkLauncherComponentData* a_pData )
{
	m_pLauncher = new AAimedSquirtProjectileLauncher();
	m_pLauncher->SetNPCControlled( TTRUE );
	m_pLauncher->SetDisableInteractions( TFALSE );
	m_pLauncher->SetOwner( a_pOwner );

	return TTRUE;
}

// $Barnyard: FUNCTION 006698c0
TBOOL ANPCMilkLauncherComponent::FireAtTarget( AUnit* a_pTarget, TFLOAT a_fHeightOffset )
{
	if ( !a_pTarget )
		return TFALSE;

	if ( !m_bFiring )
	{
		m_pLauncher->Activate();
		m_bFiring = TTRUE;
	}

	m_fFireTimer = 2.0f;

	TVector4 vTarget = *a_pTarget->GetSteer()->GetPosition();
	vTarget.y        = vTarget.y + a_fHeightOffset;

	m_pLauncher->FireAt( vTarget, TFALSE );
	return TTRUE;
}
