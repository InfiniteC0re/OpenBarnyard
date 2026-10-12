#include "pch.h"
#include "ANPCBehaviourComponent.h"
#include "ANPCBehaviourManager.h"
#include "AAnimalPopulationManager.h"
#include "ADrive.h"
#include "AGeneralUnitComponentManager.h"
#include "AUnit.h"
#include "AUnitBehaviour.h"
#include "AUnitType.h"
#include "FSM/AFSM.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"
#include "Quests/AQuestManager.h"
#include "Terrain/ATerrainInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/AInteriorSteer.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00633870
// NOTE: GetClass 00633870, CreateObject 006339a0, CreateObjectInPlace 00633a00
TDEFINE_CLASS_INIT( ANPCBehaviourComponent );

TPSTRING8_DECLARE( command );
TPSTRING8_DECLARE( schedule );

AFunctionTable ANPCBehaviourComponent::ms_oFunctionTable( &TGetClass( ANPCBehaviourComponent ), &AUnitComponent::ms_oFunctionTable );

// NOTE: retail formats the FSM names into a scratch buffer shared with other code (0x007ce238)
static TCHAR s_szNameBuffer[ 1024 ];

// $Barnyard: FUNCTION 00634630
void ANPCBehaviourComponent::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TPString8, ANPCBehaviourComponent>::Create( "GetActivityLocationName", &ANPCBehaviourComponent::GetActivityLocationName ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TPString8, ANPCBehaviourComponent>::Create( "GetActivityNextLocationName", &ANPCBehaviourComponent::GetActivityNextLocationName ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TPString8, ANPCBehaviourComponent>::Create( "GetQuestLocationName", &ANPCBehaviourComponent::GetQuestLocationName ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TPString8, ANPCBehaviourComponent>::Create( "GetSleepGoal", &ANPCBehaviourComponent::GetSleepGoal ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ANPCBehaviourComponent>::Create( "ForceVisibility", &ANPCBehaviourComponent::ForceVisibility ) );
}

// $Barnyard: FUNCTION 00633880
void ANPCBehaviourComponent::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 006338a0
// $Barnyard: FUNCTION 00633ae0
ANPCBehaviourComponent::~ANPCBehaviourComponent()
{
}

// $Barnyard: FUNCTION 00633890
AFunctionTable* ANPCBehaviourComponent::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// NOTE: folded with AUnitProfileComponent::GetManager (005d2300)
AUnitComponentManager* ANPCBehaviourComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 006338c0
void ANPCBehaviourComponent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteString( "NPC" );
	a_pBuffer->WriteInt( m_iActivity );
	a_pBuffer->WriteInt( m_iLocation );
}

// $Barnyard: FUNCTION 006338f0
void ANPCBehaviourComponent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->CheckString( "NPC" );
	a_pBuffer->ReadInt( m_iActivity );
	a_pBuffer->ReadInt( m_iLocation );
}

// $Barnyard: FUNCTION 006338b0
TBOOL ANPCBehaviourComponent::SetActivity( TINT a_iActivity )
{
	m_iActivity = a_iActivity;
	return TFALSE;
}

// $Barnyard: FUNCTION 00633d40
void ANPCBehaviourComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( m_iActivity < 0 )
		return;

	ANPCBehaviourManager*           pManager   = ANPCBehaviourManager::GetSingleton();
	ANPCBehaviourManager::Activity* pActivity  = pManager->GetActivity( m_iActivity );
	AUnit*                          pUnit      = GetOwner();
	AUnitBehaviour*                 pBehaviour = TSTATICCAST( AUnitBehaviour, pUnit->FindComponent( &TGetClass( AUnitBehaviour ) ) );

	if ( !pBehaviour )
		return;

	TBOOL     bCanStart      = TTRUE;
	TBOOL     bScheduleDrive = TFALSE;
	TPString8 strCurrentFSM;

	AFSM* pCurrentFSM = pBehaviour->GetFSMInstance()->GetCurrentMachine();

	if ( pCurrentFSM )
	{
		strCurrentFSM = pCurrentFSM->GetName();

		if ( ADrive* pDrive = pBehaviour->GetCurrentDrive() )
		{
			// Commands have priority over the schedule
			if ( pDrive->GetName() == TPS8( command ) )
				bCanStart = TFALSE;
			else
				bScheduleDrive = pDrive->GetName() == TPS8( schedule );
		}
	}

	if ( pUnit->HasInteraction() || bCanStart != TTRUE )
		return;

	// The activity's FSM, or the version of it made for the unit's type
	TPString8 strFSM( pManager->GetFSMName( pActivity->iFSM ) );

	if ( !AUnitBehaviour::FindStateMachine( strFSM ) )
		strFSM = TPS8D( "loiter_around" );

	TStringManager::String8Format( s_szNameBuffer, sizeof( s_szNameBuffer ), "%s_%s", pUnit->GetType()->GetName().GetString(), strFSM.GetString() );
	TPString8 strUnitFSM( s_szNameBuffer );

	if ( AUnitBehaviour::FindStateMachine( strUnitFSM ) )
		strFSM = strUnitFSM;

	// NOTE: not in retail, shows the activities the units get
	TINFO( "ANPCBehaviourComponent: %s gets activity %d (FSM %s, location %s)\n", pUnit->GetType()->GetName().GetString(), m_iActivity, pManager->GetFSMName( pActivity->iFSM ), pManager->GetLocationName( pActivity->iLocation ) ? pManager->GetLocationName( pActivity->iLocation ) : "none" );

	// Nothing to do if the unit is already there
	if ( bScheduleDrive && !pBehaviour->IsFSMFinished() && strCurrentFSM == strFSM && m_iLocation == pActivity->iLocation )
	{
		m_iActivity = -1;
		return;
	}

	if ( pBehaviour->SetSchedule( strFSM, TNULL, TTRUE ) == TTRUE )
	{
		m_iLocation = pActivity->iLocation;
		m_iActivity = -1;
	}
}

// $Barnyard: FUNCTION 00634250
TPString8 ANPCBehaviourComponent::GetActivityLocationName()
{
	TPString8 strLocation;

	if ( m_iLocation >= 0 )
	{
		const TCHAR* szLocation = ANPCBehaviourManager::GetSingleton()->GetLocationName( m_iLocation );

		if ( szLocation )
			strLocation = TPString8( szLocation );
	}

	return strLocation;
}

// $Barnyard: FUNCTION 00634460
TPString8 ANPCBehaviourComponent::GetActivityNextLocationName()
{
	if ( m_iActivity < 0 )
		return GetActivityLocationName();

	TPString8             strLocation;
	ANPCBehaviourManager* pManager  = ANPCBehaviourManager::GetSingleton();
	TINT                  iLocation = pManager->GetActivity( m_iActivity )->iLocation;

	if ( iLocation >= 0 )
	{
		const TCHAR* szLocation = pManager->GetLocationName( iLocation );

		if ( szLocation )
			strLocation = TPString8( szLocation );
	}

	return strLocation;
}

// $Barnyard: FUNCTION 00634050
TPString8 ANPCBehaviourComponent::GetQuestLocationName()
{
	TPString8 strLocation;
	TMatrix44 oTransform;

	AUnit* pUnit = GetOwner();

	// NOTE: retail reads the names of the locators through the collision world (0x00796300, ATerrainInterface::GetLocatorList)
	const TCHAR** ppLocatorNames = ATerrainInterface::GetSingleton()->GetLocatorList()->GetHeader()->ppNames;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	TINT           iTier         = pQuestManager->GetAct();
	TINT           iQuest        = pQuestManager->GetTier();

	// The locator of the current quest
	if ( pUnit && AAnimalPopulationManager::IsUnitInQuest( pUnit, TNULL, TFALSE ) )
	{
		ATerrainLocator* pLocator = AAnimalPopulationManager::FindQuestLocator( oTransform, 2, pUnit->GetType()->GetName().GetString(), iTier, iQuest );

		if ( pLocator )
		{
			strLocation = TPString8( ppLocatorNames[ pLocator->uiNameId ] );

			if ( !strLocation.IsEmpty() )
				return strLocation;
		}
	}

	// The default locator of the tier
	ATerrainLocator* pLocator = AAnimalPopulationManager::FindQuestLocator( oTransform, 0, pUnit->GetType()->GetName().GetString(), iTier, -1 );

	if ( pLocator )
		strLocation = TPString8( ppLocatorNames[ pLocator->uiNameId ] );

	return strLocation;
}

// $Barnyard: FUNCTION 00634320
TPString8 ANPCBehaviourComponent::GetSleepGoal()
{
	AUnit* pUnit = GetOwner();

	if ( pUnit->GetSteer()->IsA( &TGetClass( AInteriorSteer ) ) != TTRUE )
		return TPString8();

	TStringManager::String8Format( s_szNameBuffer, sizeof( s_szNameBuffer ), "%s_sleep", pUnit->GetType()->GetName().GetString() );
	TPString8 strGoal( s_szNameBuffer );

	if ( TSTATICCAST( AInteriorSteer, pUnit->GetSteer() )->HasGoal( strGoal ) != TTRUE )
		return TPString8();

	return strGoal;
}

// $Barnyard: FUNCTION 00633b00
void ANPCBehaviourComponent::ForceVisibility()
{
	ANPCBehaviourManager::GetSingleton()->ForceVisibility( GetOwner() );
}
