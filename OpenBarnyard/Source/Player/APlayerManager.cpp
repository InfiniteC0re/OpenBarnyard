#include "pch.h"
#include "APlayerManager.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitPlayer.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: TClass 0x007b48dc, retail registers it with TTask as the parent class although it's a TObject
// NOTE: GetClass 0061fab0, CreateObject 0061ff30, CreateObjectInPlace 0061ff90
TDEFINE_CLASS_INIT( APlayerManager );

// DAT_007b48ac
AFunctionTable APlayerManager::ms_oFunctionTable( &TGetClass( APlayerManager ), TNULL );

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 0061fb40
static TObject* __stdcall APlayerManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return APlayerManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 0061ffd0
void APlayerManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &APlayerManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<AUnit*, APlayerManager, TINT>::Create( "GetCurPlayer", &APlayerManager::GetPlayerUnit ) );
}

// $Barnyard: FUNCTION 0061fac0
void APlayerManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0061fe40
APlayerManager::APlayerManager()
{
	m_pPlayerSets = &m_aPlayerSets[ PLAYERSET_SIM ];

	// The player units are assigned to the slots of the current player set once they are created
	AUnitManager* pUnitManager = AUnitManager::GetSingleton();
	m_oUnitCreatedListener.Connect( pUnitManager->GetUnitCreatedEmitter(), this, TREINTERPRETCAST( Toshi::TGenericListener::t_Callback, OnUnitCreated ), 0 );
	m_oUnitsRemovedListener.Connect( pUnitManager->GetUnitsRemovedEmitter(), this, TREINTERPRETCAST( Toshi::TGenericListener::t_Callback, OnUnitsRemoved ), 0 );
}

// $Barnyard: FUNCTION 0061fba0
// $Barnyard: FUNCTION 0061ff10
APlayerManager::~APlayerManager()
{
	m_oUnitCreatedListener.Disconnect();
	m_oUnitsRemovedListener.Disconnect();
}

// $Barnyard: FUNCTION 0061fc80
TBOOL __stdcall APlayerManager::OnUnitCreated( APlayerManager* a_pCaller, void* a_pOwner, AUnit** a_ppUnit )
{
	if ( ( *a_ppUnit )->GetClass() == &TGetClass( AUnitPlayer ) )
		a_pCaller->m_pPlayerSets->SetPlayerUnit( *a_ppUnit, -1 );

	return TTRUE;
}

// $Barnyard: FUNCTION 0061fcb0
TBOOL __stdcall APlayerManager::OnUnitsRemoved( APlayerManager* a_pCaller, void* a_pOwner, TUINT* a_pType )
{
	if ( *a_pType <= 2 )
	{
		for ( TINT i = 0; i < TARRAYSIZE( a_pCaller->m_aPlayerSets ); i++ )
			a_pCaller->m_aPlayerSets[ i ].ResetPlayerUnits();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0061fb00
TINT APlayerManager::GetNextPlayer()
{
	if ( ++m_iCurrentPlayerIt >= m_pPlayerSets->GetHumanPlayerCount() )
		m_iCurrentPlayerIt = 0;

	return m_iCurrentPlayerIt;
}

// $Barnyard: FUNCTION 0061fae0
AUnit* APlayerManager::GetPlayer( TINT a_iIndex )
{
	return m_pPlayerSets->GetHumanPlayerUnit( a_iIndex );
}

// $Barnyard: FUNCTION 0061faf0
AUnit* APlayerManager::GetPlayerUnit( TINT a_iSlot )
{
	return m_pPlayerSets->GetPlayerUnit( a_iSlot );
}

static const TCHAR* s_aPlayerSetNames[ APlayerManager::PLAYERSET_NUMOF ] = {
	"sim",
	"minigame"
};

// $Barnyard: FUNCTION 0061fb50
APlayerManager::PLAYERSET APlayerManager::GetPlayerSetIndex( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < PLAYERSET_NUMOF; i++ )
	{
		if ( a_rcName.GetString8().CompareNoCase( s_aPlayerSetNames[ i ] ) == 0 )
			return i;
	}

	return -1;
}
