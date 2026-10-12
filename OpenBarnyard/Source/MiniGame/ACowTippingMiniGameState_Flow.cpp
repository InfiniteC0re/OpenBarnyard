#include "pch.h"
#include <Render/TCameraObject.h> // before the Physics headers (Opcode #defines PI)
#include "ACowTippingMiniGameState.h"
#include "GameInterface/ACreditsState.h"
#include "Units/Projectiles/AProjectile.h"
#include "AMiniGameManager.h"
#include "ARandom.h"
#include "AOptions.h"
#include "ALoadScreen.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/AWeatherManager.h"
#include "Helpers/AGameLoader.h"
#include "FSM/AMiscFunctions.h"
#include "Helpers/AInstanceManager.h"
#include "Helpers/ATreeManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Units/AUnitManager.h"
#include "Units/AAnimatedUnit.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/SaveLoadSKU.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AVOStream.h"
#include "Sound/APositionalSoundSet.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AMilkManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Tasks/ARootTask.h"
#include "GUI/AGUI2.h"
#include "Input/AInputHandler.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Render/ARenderer.h"

#include <Input/TInputDeviceMouse.h>
#include <Math/TMatrix44.h>
#include <Math/TQuaternion.h>
#include <Math/TRandom.h>
#include <Toshi/TScheduler.h>
#include "Units/AKinematicsObstruction.h"
#include "SoundBank/dagboss.h"

// NOTE: Opcode (physics headers) #defines PI
#undef PI

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( up );
TPSTRING8_DECLARE( uptawnt );
TPSTRING8_DECLARE( duck );
TPSTRING8_DECLARE( duck_up );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( distress_pleased );
TPSTRING8_DECLARE( cowtipping_tomato );

// NOTE: ACowTippingMiniGameState is split over several .cpp files, this one has the [Flow] part.

//-----------------------------------------------------------------------------
// Helpers for provisional fields
//-----------------------------------------------------------------------------

// NOTE: several provisional TINT fields hold floats, pointers or lists in retail.
#define CT_FLOAT( FIELD ) ( *TREINTERPRETCAST( TFLOAT*, &( FIELD ) ) )

// NOTE: the lists at 0x13120, 0x13124, 0x13268, 0x1326c, 0x13270 and 0x13274 are T2GenericSLists
// (circular, the node is the first member of Enemy/Raccoon), their fields are still TINTs.
template <class T>
static T2GenericSList& CT_SList( Toshi::T2SList<T>& a_rList )
{
	return a_rList;
}

template <class T>
static T* CT_Node( T2GenericSList::Node* a_pNode )
{
	return TREINTERPRETCAST( T*, a_pNode );
}

template <class T>
static T2GenericSList::Node* CT_AsNode( T* a_pItem )
{
	return TREINTERPRETCAST( T2GenericSList::Node*, a_pItem );
}

// Bit arrays at 0x57ac, 0x57b0, 0x57b4, 0x57b8 and 0x57bc (m_auiUsedRaccoonBits...)
static TBOOL CT_IsBitSet( const TUINT8* a_puiBits, TINT a_iBit )
{
	return ( a_puiBits[ TUINT( a_iBit ) >> 3 ] & ( 1 << ( a_iBit & 7 ) ) ) != 0;
}

static void CT_SetBit( TUINT8* a_puiBits, TINT a_iBit )
{
	a_puiBits[ TUINT( a_iBit ) >> 3 ] |= 1 << ( a_iBit & 7 );
}

// Member of the state at a byte offset that isn't a named field of the header yet
template <class T>
static T* CT_At( ACowTippingMiniGameState* a_pThis, TUINT a_uiOffset )
{
	return TREINTERPRETCAST( T*, TREINTERPRETCAST( TUINT8*, a_pThis ) + a_uiOffset );
}

// Crate[ 6 ][ 5 ] at 0x126a8
static ACowTippingMiniGameState::Crate* CT_GetCrate( ACowTippingMiniGameState* a_pThis, TINT a_iGroup, TINT a_iSlot )
{
	return &a_pThis->m_aCrates[ a_iGroup ][ a_iSlot ];
}

// Coin[ 8 ] at 0x13128
static ACowTippingMiniGameState::Coin* CT_GetCoin( ACowTippingMiniGameState* a_pThis, TINT a_iIndex )
{
	return &a_pThis->m_aCoins[ a_iIndex ];
}

// Weighted random picker that PickWeighted picks from: a TRandom, the total weight
// and a T2Vector of { value, weight }. Retail builds it on the stack with N = 30, 10, 14 or 8.
template <TINT N>
struct CT_RandomPicker
{
	struct Item
	{
		TINT iValue;
		TINT iWeight;
	};

	TRandom* pRandom;
	TINT     iTotalWeight;
	TINT     iNumItems;
	Item     aItems[ N ];

	// $Barnyard: FUNCTION 004ab100
	// NOTE: 004ab100 is the constructor of the T2Vector with 10 items, the others are inlined
	CT_RandomPicker()
	    : pRandom( &ARandom::GetSingleton()->m_oRandom ), iTotalWeight( 0 ), iNumItems( 0 )
	{
		for ( TINT i = 0; i < N; i++ )
		{
			aItems[ i ].iValue  = 0;
			aItems[ i ].iWeight = 0;
		}
	}

	void Add( TINT a_iValue )
	{
		iTotalWeight++;
		aItems[ iNumItems ].iValue  = a_iValue;
		aItems[ iNumItems ].iWeight = 1;
		iNumItems++;
	}

	TINT Pick()
	{
		return *TREINTERPRETCAST( TINT*, ACowTippingMiniGameState::PickWeighted( this ) );
	}
};

static TBOOL CT_IsMode23( TINT a_iMode )
{
	return a_iMode == ACowTippingMiniGameState::Stage::STAGETYPE_JUNKYARD_BOSS || a_iMode == ACowTippingMiniGameState::Stage::STAGETYPE_JUNKYARD;
}

//-----------------------------------------------------------------------------
// Level script commands
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00708ca0
// NOTE: 00708ca0 is the dynamic initialiser of this table (00782f30)
ACowTippingMiniGameState::LevelCommandFn ACowTippingMiniGameState::ms_apfnLevelCommands[ NUM_LEVEL_COMMANDS ] = {
	&ACowTippingMiniGameState::Cmd_WaitTime,
	&ACowTippingMiniGameState::Cmd_WaitEnemyCount,
	&ACowTippingMiniGameState::Cmd_WaitActiveRaccoonCount,
	&ACowTippingMiniGameState::Cmd_WaitCrateCount,
	&ACowTippingMiniGameState::Cmd_WaitAllCounts,
	&ACowTippingMiniGameState::Cmd_SpawnEnemies,
	&ACowTippingMiniGameState::Cmd_ActivateRaccoon,
	&ACowTippingMiniGameState::Cmd_ActivateGopher,
	&ACowTippingMiniGameState::Cmd_RandomJump,
	&ACowTippingMiniGameState::Cmd_JumpIfScore,
	&ACowTippingMiniGameState::Cmd_EndLevelWhenClear,
	&ACowTippingMiniGameState::Cmd_EndLevel,
	&ACowTippingMiniGameState::Cmd_SetLevelTime,
	&ACowTippingMiniGameState::Cmd_NextStage,
	&ACowTippingMiniGameState::Cmd_RaccoonsShootNow,
	&ACowTippingMiniGameState::Cmd_JumpIfObjectsLeft,
	&ACowTippingMiniGameState::Cmd_PlaceCoins,
	&ACowTippingMiniGameState::Cmd_SpawnEnemiesAlt,
};

// Runs the commands of the level script of the current player and stage until one returns TFALSE
// (at most 80 more commands in a frame).
// $Barnyard: FUNCTION 004aaae0
TINT ACowTippingMiniGameState::RunLevelScript( TINT a_iUnk1 )
{
	TINT iLevel = m_aPlayers[ m_iCurrentPlayer ].iLevel;

	if ( iLevel >= m_iNumLevels )
		return iLevel;

	LevelScript* pScript = &m_aLevels[ iLevel ].aScripts[ m_aStages[ m_iStage ].iScript ];

	const LevelCommand* pCommand = &pScript->aCommands[ m_iScriptCommand ];
	TBOOL               bResult  = ( this->*ms_apfnLevelCommands[ pCommand->uiType ] )( pCommand );

	for ( TINT i = 0; bResult; i++ )
	{
		if ( i >= 80 )
			break;

		if ( ++m_iScriptCommand >= pScript->iNumCommands )
		{
			m_iScriptCommand = 0;
			m_iScriptLoops++;
		}

		pCommand = &pScript->aCommands[ m_iScriptCommand ];
		bResult  = ( this->*ms_apfnLevelCommands[ pCommand->uiType ] )( pCommand );
	}

	return bResult;
}

// 0: waits until fValue seconds have passed since the last time this command passed
// $Barnyard: FUNCTION 004a7740
TBOOL ACowTippingMiniGameState::Cmd_WaitTime( const LevelCommand* a_pCommand )
{
	TFLOAT fTime = g_oSystemManager.GetScheduler()->GetTotalTime();

	if ( fTime - m_fScriptWaitStart <= a_pCommand->fValue )
		return TFALSE;

	m_fScriptWaitStart = fTime;
	return TTRUE;
}

// 1: passes while the number of the Enemies of type uiParam1 (0xff: all) is at most fValue
// $Barnyard: FUNCTION 004acf50
TBOOL ACowTippingMiniGameState::Cmd_WaitEnemyCount( const LevelCommand* a_pCommand )
{
	TINT iCount = ( a_pCommand->uiParam1 == 255 ) ? GetNumActiveEnemies() : GetNumActiveEnemiesOfType( a_pCommand->uiParam1 );
	return TFLOAT( iCount ) <= a_pCommand->fValue;
}

// 2: passes while list 0x13268 has at most fValue items
// $Barnyard: FUNCTION 004aabb0
TBOOL ACowTippingMiniGameState::Cmd_WaitActiveRaccoonCount( const LevelCommand* a_pCommand )
{
	return TFLOAT( CT_SList( m_llActiveRaccoons ).Size() ) <= a_pCommand->fValue;
}

// 3: passes while group uiParam1 (0xff: all) has at most fValue used Crates
// $Barnyard: FUNCTION 004a7780
TBOOL ACowTippingMiniGameState::Cmd_WaitCrateCount( const LevelCommand* a_pCommand )
{
	TINT iCount = ( a_pCommand->uiParam1 == 255 ) ? CountActivePickups() : GetNumUsedCrates( a_pCommand->uiParam1 );
	return TFLOAT( iCount ) <= a_pCommand->fValue;
}

// 4: Cmd_WaitEnemyCount and Cmd_WaitActiveRaccoonCount
// $Barnyard: FUNCTION 004acfa0
TBOOL ACowTippingMiniGameState::Cmd_WaitAllCounts( const LevelCommand* a_pCommand )
{
	if ( a_pCommand->uiParam1 == 255 )
	{
		if ( TFLOAT( GetNumActiveEnemies() ) > a_pCommand->fValue )
			return TFALSE;
	}
	else if ( TFLOAT( GetNumActiveEnemiesOfType( a_pCommand->uiParam1 ) ) > a_pCommand->fValue )
	{
		return TFALSE;
	}

	return TFLOAT( CT_SList( m_llActiveRaccoons ).Size() ) <= a_pCommand->fValue;
}

// 5 and 17: spawns fValue Enemies (SpawnEnemy)
static TBOOL CT_SpawnEnemies( ACowTippingMiniGameState* a_pThis, const ACowTippingMiniGameState::LevelCommand* a_pCommand, TBOOL a_bUnk )
{
	if ( a_pThis->m_eTurnEnd != ACowTippingMiniGameState::TURNEND_NONE )
		return TFALSE;

	TINT  iMode    = a_pThis->m_aStages[ a_pThis->m_iStage ].iStageType;
	TBOOL bResult  = TTRUE;
	TINT  iCount   = TINT( a_pCommand->fValue );
	TBOOL bVariant = TFALSE;

	if ( iCount > 0 )
	{
		while ( bResult )
		{
			TINT iType = a_pCommand->uiParam1;

			if ( iType == 255 )
				iType = a_pThis->GetRandomFreeEnemy()->eType;

			TINT iIndex;

			if ( a_pCommand->cParam2 == 'r' )
			{
				if ( CT_IsMode23( iMode ) )
					bVariant = TFALSE;

				iIndex = a_pThis->GetRandomStartIndex( &bVariant );
			}
			else
			{
				iIndex = a_pCommand->uiParam3;

				// NOTE: retail also calls strlen on cParam2 here and ignores the result
				bVariant = a_pCommand->cParam2 != 'a';
			}

			if ( iMode == ACowTippingMiniGameState::Stage::STAGETYPE_RACCOONS )
				iType = ACowTippingMiniGameState::Enemy::ENEMYTYPE_RACCOON;
			else if ( CT_IsMode23( iMode ) )
				iType = 4;

			bResult &= a_pThis->SpawnEnemy( iType, iIndex, bVariant, a_bUnk );

			if ( --iCount <= 0 )
				break;
		}
	}

	return TTRUE;
}

// 5
// $Barnyard: FUNCTION 004c3410
TBOOL ACowTippingMiniGameState::Cmd_SpawnEnemies( const LevelCommand* a_pCommand )
{
	return CT_SpawnEnemies( this, a_pCommand, TFALSE );
}

// 17
// $Barnyard: FUNCTION 004c3530
TBOOL ACowTippingMiniGameState::Cmd_SpawnEnemiesAlt( const LevelCommand* a_pCommand )
{
	return CT_SpawnEnemies( this, a_pCommand, TTRUE );
}

// 6: activates fValue Raccoons of list 0x1326c ('r': random index) or the one with index uiParam3
// $Barnyard: FUNCTION 004bc9c0
TBOOL ACowTippingMiniGameState::Cmd_ActivateRaccoon( const LevelCommand* a_pCommand )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return TFALSE;

	TBOOL bResult = TTRUE;
	TINT  iCount  = TINT( a_pCommand->fValue );
	TINT  iType   = a_pCommand->uiParam1;

	if ( a_pCommand->cParam2 != 'r' )
		return ActivateRaccoon( a_pCommand->uiParam3, iType ) & 1;

	while ( iCount > 0 && bResult )
	{
		TBOOL bActivated = TFALSE;

		if ( m_eTurnEnd == TURNEND_NONE && CT_SList( m_llFreeRaccoons ).Begin() != CT_SList( m_llFreeRaccoons ).End() )
		{
			TINT iIndex = GetRandomRaccoonIndex();

			if ( iIndex > -1 )
				bActivated = ActivateRaccoon( iIndex, iType );
		}

		bResult &= bActivated;
		iCount--;
	}

	return bResult;
}

// 7: activates fValue Raccoons of group uiParam1 (0xff: random, 0xfe: PlaceWildMikeBox( cParam2 ))
// $Barnyard: FUNCTION 004bca60
TBOOL ACowTippingMiniGameState::Cmd_ActivateGopher( const LevelCommand* a_pCommand )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return TFALSE;

	TBOOL bResult = TTRUE;
	TINT  iCount  = TINT( a_pCommand->fValue );

	if ( iCount > 0 )
	{
		while ( bResult )
		{
			TINT iGroup = a_pCommand->uiParam1;

			if ( iGroup == 255 )
				iGroup = GetRandomCrateGroup();

			if ( iGroup == 254 )
				return PlaceWildMikeBox( a_pCommand->cParam2 );

			bResult &= ActivateGopher( iGroup );

			if ( --iCount <= 0 )
				return TTRUE;
		}
	}

	return TTRUE;
}

// 8: jumps to command cParam2 with the chance fValue
// $Barnyard: FUNCTION 004aabe0
TBOOL ACowTippingMiniGameState::Cmd_RandomJump( const LevelCommand* a_pCommand )
{
	if ( ARandom::GetSingleton()->m_oRandom.GetFloat() <= a_pCommand->fValue )
		m_iScriptCommand = a_pCommand->cParam2;

	return TTRUE;
}

// 9: jumps to command cParam2 if the score of the player is at least fValue
// $Barnyard: FUNCTION 004aac50
TBOOL ACowTippingMiniGameState::Cmd_JumpIfScore( const LevelCommand* a_pCommand )
{
	if ( m_aPlayers[ m_iCurrentPlayer ].iScore >= TINT( a_pCommand->fValue ) )
		m_iScriptCommand = a_pCommand->cParam2;

	return TTRUE;
}

// 10: once the script has looped fValue times, waits for Cmd_WaitAllCounts( any, 0 ) and ends the level
// $Barnyard: FUNCTION 004ad010
TBOOL ACowTippingMiniGameState::Cmd_EndLevelWhenClear( const LevelCommand* a_pCommand )
{
	if ( TFLOAT( m_iScriptLoops + 1 ) < a_pCommand->fValue )
		return TTRUE;

	// NOTE: retail leaves uiType/cParam2/uiParam3 of the temporary uninitialised
	LevelCommand oCommand;
	oCommand.uiType   = 0;
	oCommand.uiParam1 = 255;
	oCommand.cParam2  = 0;
	oCommand.uiParam3 = 0;
	oCommand.fValue   = 0.0f;

	if ( Cmd_WaitAllCounts( &oCommand ) )
		m_bLevelDone = TTRUE;

	return TFALSE;
}

// 11: once the script has looped fValue times, ends the level
// $Barnyard: FUNCTION 004a7800
TBOOL ACowTippingMiniGameState::Cmd_EndLevel( const LevelCommand* a_pCommand )
{
	if ( TFLOAT( m_iScriptLoops + 1 ) < a_pCommand->fValue )
		return TTRUE;

	m_bLevelDone = TTRUE;
	return TFALSE;
}

// 12: sets 0x13350 and 0x13314 to fValue
// $Barnyard: FUNCTION 004a7830
TBOOL ACowTippingMiniGameState::Cmd_SetLevelTime( const LevelCommand* a_pCommand )
{
	if ( m_fLevelTime != a_pCommand->fValue )
	{
		m_fLevelTime  = a_pCommand->fValue;
		m_fLevelTimer = a_pCommand->fValue;
	}

	return TTRUE;
}

// 13: goes to the next stage
// $Barnyard: FUNCTION 004bcae0
TBOOL ACowTippingMiniGameState::Cmd_NextStage( const LevelCommand* a_pCommand )
{
	m_iStage = TMath::Min( m_iStage + 1, m_iNumStages );

	m_iScriptCommand                      = 0;
	m_iScriptLoops                        = 0;
	m_aPlayers[ m_iCurrentPlayer ].iLevel = 0;

	SendAllUnitsBackToStart();
	m_fReticleTimer = m_fReticleTime;
	SetupLifeHuds();
	m_oTargetReticle.SetActive( TFALSE, TFALSE, 0.0f );

	return TFALSE;
}

// 14
// $Barnyard: FUNCTION 004a7860
TBOOL ACowTippingMiniGameState::Cmd_RaccoonsShootNow( const LevelCommand* a_pCommand )
{
	m_bRaccoonsShootNow = TTRUE;
	return TTRUE;
}

// 15: jumps to command cParam2 if 0x133c8 is at most fValue
// $Barnyard: FUNCTION 004a77d0
TBOOL ACowTippingMiniGameState::Cmd_JumpIfObjectsLeft( const LevelCommand* a_pCommand )
{
	if ( TFLOAT( m_iObjectsLeft ) <= a_pCommand->fValue )
		m_iScriptCommand = a_pCommand->cParam2;

	return TTRUE;
}

// 16: places fValue Coins at the positions at 0x12dc ('r': random free one, otherwise uiParam3)
// $Barnyard: FUNCTION 004c21c0
TBOOL ACowTippingMiniGameState::Cmd_PlaceCoins( const LevelCommand* a_pCommand )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return TFALSE;

	TBOOL bResult = TTRUE;
	TINT  iCount  = TINT( a_pCommand->fValue );

	if ( iCount > 0 )
	{
		while ( bResult )
		{
			TINT iIndex = ( a_pCommand->cParam2 == 'r' ) ? GetRandomCoinIndex() : a_pCommand->uiParam3;

			if ( iIndex < 0 )
				return TFALSE;

			CT_SetBit( m_auiUsedCoinBits, iIndex );
			bResult &= TakeCoin( m_aCoinPositions + iIndex ) != TNULL;

			if ( --iCount <= 0 )
				return TTRUE;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// AGameState
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004bdd00
TBOOL ACowTippingMiniGameState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_fTurnEndTimer != 0.0f && a_eInputCommand == AInputCommand_Quit )
	{
		m_fTurnEndTimer = 0;
		return AMiniGameState::ProcessCommand( AInputCommand_Quit, a_pInputEvent, a_rStopEvents );
	}

	// DOES_NOTHING-style stub 006528c0 that returns TFALSE
	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) && a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		return TFALSE;

	if ( !m_oStartHudElement.IsForceHidden() )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( ARootTask::GetSingleton()->IsPaused() )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( m_bGameOverPending == TTRUE )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( m_eTurnEnd != TURNEND_NONE && a_eInputCommand == AInputCommand_Shoot )
	{
		TInputInterface::EVENT_TYPE eType = a_pInputEvent->GetEventType();

		if ( eType == TInputInterface::EVENT_TYPE_GONE_DOWN || eType == TInputInterface::EVENT_TYPE_REPEAT )
			return AMiniGameState::ProcessCommand( AInputCommand_Shoot, a_pInputEvent, a_rStopEvents );
	}

	TINT iSlot = APlayerManager::GetSingleton()->GetMiniGamePlayerSet()->GetPlayerSlotIndexByDevice( a_pInputEvent->GetSource() );
	if ( iSlot == m_iCurrentPlayer )
	{
		TInputInterface::EVENT_TYPE eType  = a_pInputEvent->GetEventType();
		TBOOL                       bDown  = eType == TInputInterface::EVENT_TYPE_GONE_DOWN || eType == TInputInterface::EVENT_TYPE_REPEAT;
		TINT                        iStage = m_aStages[ m_iStage ].iCameraSettings;

		switch ( a_eInputCommand )
		{
			case AInputCommand_Move:
				if ( a_pInputEvent->GetSource()->IsA( &TGetClass( TInputDeviceMouse ) ) )
				{
					TFLOAT fSensitivity = AOptions::GetSingleton()->GetSensitivity();
					m_fMouseX           = a_pInputEvent->GetMagnitudeFloat( 0 ) * fSensitivity;

					fSensitivity = AOptions::GetSingleton()->GetSensitivity();
					m_fMouseY    = a_pInputEvent->GetMagnitudeFloat( 1 ) * fSensitivity;

					TScheduler* pScheduler = g_oSystemManager.GetScheduler();

					if ( 0.0f == m_fUnk_13304 )
						m_fUnk_13304 = pScheduler->GetTotalTime();

					TFLOAT fDistance = TMath::Abs( m_vPlayerPos.x - m_vPlayerCentre.x );
					m_fInputX        = 0.0f;
					m_fMoveY         = 0.0f;

					TFLOAT fDistanceFactor = ( fDistance * 0.3f ) * ( fDistance * 0.3f ) * ( fDistance * 0.3f );

					if ( fDistanceFactor < 1.0f )
						fDistanceFactor = 1.0f;

					TFLOAT fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
					TFLOAT fScaleX    = 800.0f / AGUI2::GetRootElement()->GetWidth();
					TFLOAT fScaleY    = 600.0f / AGUI2::GetRootElement()->GetHeight();
					TFLOAT fInvDelta  = 1.0f / fDeltaTime;
					TFLOAT fX         = TMath::Pow( TMath::Abs( m_fMouseX ), m_fMouseExponentX ) * fInvDelta * fScaleX;
					TFLOAT fY         = TMath::Pow( TMath::Abs( m_fMouseY ), m_fMouseExponentY ) * fInvDelta * fScaleY;
					TFLOAT fMoveX     = fX;

					if ( fX < m_fMouseMinMoveX && fMoveX < fY )
						fMoveX = 0.0f;

					if ( fMoveX > m_fMouseMaxMoveX )
						fMoveX = m_fMouseMaxMoveX;

					if ( fY > m_fMouseMaxMoveY )
						fY = m_fMouseMaxMoveY;

					if ( m_fMouseX < 0.0f )
						fMoveX = -fMoveX;

					if ( m_fMouseY < 0.0f )
						fY = -fY;

					TFLOAT fFrameScale = fDeltaTime * 0.01f;

					// Moves the target along the direction at 0x10ec
					TVector4 vMove = m_vRailDirection;
					vMove.Multiply( fFrameScale * fMoveX );
					vMove.Multiply( m_fMoveSpeed );
					vMove.Multiply( fDistanceFactor );
					vMove.Multiply( m_fMouseMoveSpeed );

					TVector3& rPosition = *TREINTERPRETCAST( TVector3*, &m_vPlayerPos );
					rPosition.x += vMove.x;
					rPosition.y += vMove.y;
					rPosition.z += vMove.z;

					m_bUnk_1340A = TTRUE;
					m_bNoInputX  = TTRUE;
					m_bUnk_13410 = TFALSE;
					m_fCameraStartPitch -= fFrameScale * fY * m_fMousePitchSpeed * m_fPitchInputSpeed;

					UpdatePlayerTransform( 0.0f, 0 );
				}
				else
				{
					TFLOAT fX = a_pInputEvent->GetMagnitudeFloat( 0 );
					TFLOAT fY = a_pInputEvent->GetMagnitudeFloat( 1 );

					if ( TMath::Abs( fX ) > 0.1f || TMath::Abs( fY ) > 0.1f )
						m_bUnk_13410 = TTRUE;

					if ( m_bUnk_13410 )
					{
						m_fInputX = fX;

						if ( AOptions::GetSingleton()->IsMouseInverted() )
							fY = -fY;

						m_fMoveY     = fY;
						m_bUnk_1340A = TFALSE;
					}
				}
				break;
			case AInputCommand_Up:
				SelectPrevious( iSlot != 1 );
				return TTRUE;
			case AInputCommand_Down:
				SelectNext( iSlot != 1 );
				return TTRUE;
			case AInputCommand_LeftWithUp:
				if ( bDown )
				{
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
					m_fInputX = -1.0f;
				}
				else
				{
					if ( m_fInputX < 0.0f )
					{
						m_fInputX      = 0.0f;
						m_bNoInputX    = TTRUE;
						m_fAccelTimerX = TMath::MAXFLOAT;
					}

					m_bUnk_13404 = TTRUE;
				}

				m_bUnk_1340A = TFALSE;
				m_bUnk_13410 = TFALSE;
				return TTRUE;
			case AInputCommand_RightWithUp:
				if ( bDown )
				{
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
					m_fInputX = 1.0f;
				}
				else
				{
					m_bUnk_13404 = TTRUE;

					if ( m_fInputX > 0.0f )
					{
						m_fInputX      = 0.0f;
						m_bNoInputX    = TTRUE;
						m_fAccelTimerX = TMath::MAXFLOAT;
					}
				}

				m_bUnk_1340A = TFALSE;
				m_bUnk_13410 = TFALSE;
				return TTRUE;
			case AInputCommand_ForwardWithUp:
				if ( bDown )
				{
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
					m_fMoveY = -1.0f;
				}
				else if ( m_fMoveY < 0.0f )
				{
					m_fMoveY       = 0.0f;
					m_bNoInputY    = TTRUE;
					m_fAccelTimerY = TMath::MAXFLOAT;
				}

				m_bUnk_1340A = TFALSE;
				m_bUnk_13410 = TFALSE;
				break;
			case AInputCommand_BackwardWithUp:
				if ( bDown )
				{
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
					m_fMoveY = 1.0f;
				}
				else if ( m_fMoveY > 0.0f )
				{
					m_fMoveY       = 0.0f;
					m_bNoInputY    = TTRUE;
					m_fAccelTimerY = TMath::MAXFLOAT;
				}

				m_bUnk_1340A = TFALSE;
				m_bUnk_13410 = TFALSE;
				break;
			case AInputCommand_Shoot:
				if ( !IsPowerUpActive( 2 ) )
					m_bUnk_13408 = TFALSE;

				if ( IsPlayerDucking() || m_fThrowCooldown >= 0.0f || m_fReticleTimer >= 0.0f || !bDown )
					return TTRUE;

				if ( IsPowerUpActive( 2 ) )
				{
					if ( m_bUnk_13408 )
					{
						m_bUnk_13408 = TTRUE;
						return TTRUE;
					}

					if ( eType == TInputInterface::EVENT_TYPE_GONE_DOWN )
					{
						m_iMaxAmmo = TMath::Max( m_oMilkReticleManager.GetNumTargets(), 1 ) * m_iAmmoPerTarget;
						m_iMaxAmmo = TMath::Min( m_iMaxAmmoLimit, m_iMaxAmmo );
						SetAmmoCount( 0, m_iMaxAmmo );
					}
				}
				else
				{
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
				}

				StartThrow();
				m_bUnk_13408 = TTRUE;
				return TTRUE;
			case AInputCommand_SideStep:
			{
				TFLOAT fValue = a_pInputEvent->GetMagnitudeFloat( 0 );
				TFLOAT fSign  = ( fValue <= 0.0f ) ? -1.0f : 1.0f;

				m_fCameraYawDelta = fSign * m_aCameraSettings[ iStage ].fSideStepSpeed * fValue * fValue;
				m_bSideStepping   = fValue >= 0.01f || fValue <= -0.01f;
				break;
			}
			case AInputCommand_Reload:
				if ( !IsPlayerDucking() && m_fThrowCooldown < 0.0f )
					StartReload();
				break;
			case AInputCommand_Dodge:
			{
				AAnimatableObject* pObject = m_pLauncher;

				if ( eType != TInputInterface::EVENT_TYPE_GONE_DOWN )
					pObject->PlayAnimation( TPS8( duck_up ) );
				else
					pObject->PlayAnimation( TPS8( duck ) );

				break;
			}
			default:
				break;
		}
	}

	if ( m_fInputX == 0.0f && m_fMoveY == 0.0f )
		m_pLockUnit = 0;

	return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 004a7a40
void ACowTippingMiniGameState::OnRender( TFLOAT a_fDeltaTime )
{
	m_oMilkReticleManager.Update( m_fGameTime, TTRUE );
	m_oTargetReticle.Update( m_fGameTime );

	if ( CT_IsMode23( m_aStages[ m_iStage ].iStageType ) && AMilkManager::GetSingleton() )
	{
		// NOTE: retail passes a_fDeltaTime to AMilkManager::Render (00667d70, RET 4)
		AMilkManager::GetSingleton()->Render();
	}

	m_fGameTime = 0.0f;
}

// $Barnyard: FUNCTION 004a79f0
ASoundId ACowTippingMiniGameState::GetSound()
{
	return m_bStoryEnding ? ASOUNDID_INVALID : 34;
}

// $Barnyard: FUNCTION 004a7ab0
void ACowTippingMiniGameState::OnPause()
{
	if ( m_aStages[ m_iStage ].iCameraSettings != 1 && m_aStages[ m_iStage ].iStageType != Stage::STAGETYPE_RACCOONS )
		m_pLauncher->SetSkeletonUpdating( TTRUE, TTRUE );

	AMiniGameState::OnPause();
}

// $Barnyard: FUNCTION 004a7a10
TFLOAT ACowTippingMiniGameState::GetFOV()
{
	return m_aCameraSettings[ m_aStages[ m_iStage ].iCameraSettings ].fFOV * 0.017453292f;
}

// $Barnyard: FUNCTION 004c3650
void ACowTippingMiniGameState::OnActivate()
{
	AInputHandler::GetSingleton()->SetRumblePaused( TFALSE );

	if ( m_bStoryEnding )
	{
		AMusicManager::GetSingleton()->SetForcedSound( ASOUNDID_INVALID );
		AMusicManager::GetSingleton()->ForceUpdate();
		AMiniGameState::OnActivate();

		if ( m_bGameOver )
		{
			switch ( m_iGameOverStep )
			{
				case 1:
					AGameStateController::GetSingleton()->PushState( new ACreditsState( TTRUE, TTRUE ) );
					break;
				case 2:
				{
					g_oLoadScreen.StartLoading( 100, TTRUE );
					AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
					AGameTimeFXManager::GetSingleton()->ClearForcedFX();
					AGameTimeFXManager::GetSingleton()->SetDayPhase( AGameTimeManager::DAYPHASE_AFTERNOON, TFALSE, 0.0f );
					AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );
					AVOStream::GetSingleton()->Close( TTRUE );
					AVOStream::GetSingleton()->Open( "data/sound/FSB/ss.fsb", TTRUE, TFALSE );
					g_oLoadScreen.Update( 1.0f, TTRUE );
					AGameLoader::Terrain_Set( AGameLoader::Terrain_ss11, TFALSE, TTRUE, 0, 0, 0, 0 );
					g_oLoadScreen.Update( 1.0f, TTRUE );
					AGameLoader::Terrain_Load();
					g_oLoadScreen.Update( 1.0f, TTRUE );
					g_oLoadScreen.Reset();
					AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
					AInstanceManager::GetSingleton()->Reset();

					AGameStateController::GetSingleton()->PushState(
					    new ASSeqState( "SS11_prolog", "SS11", "SS11_cameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
					);
					break;
				}
				case 3:
				{
					APlayerProgress* pProgress = APlayerProgress::GetSingleton();
					AGameTimeFXManager::GetSingleton()->SetDayPhase( AGameTimeManager::DAYPHASE_AFTERNOON, TFALSE, 0.0f );
					AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );

					if ( pProgress->GetCurrentSlot() )
					{
						pProgress->SetCurrentSlotHero();

						SaveLoadSKU* pSaveLoad = new SaveLoadSKU( pProgress, TTRUE, TNULL );
						pSaveLoad->StartSave( TFALSE, TFALSE, TFALSE );
						AGameStateController::GetSingleton()->PushState( pSaveLoad );
					}
					else
					{
						AMiniGameManager::GetSingleton()->RequestReturn();
					}

					break;
				}
				case 4:
				{
					APlayerProgress* pProgress   = APlayerProgress::GetSingleton();
					TBOOL            bUnk2       = pProgress->IsChampion();
					TBOOL            bBarnDetail = pProgress->HasCurrentSlotBarnDetail();
					TINT             iMessageLocale;

					if ( bUnk2 )
						iMessageLocale = bBarnDetail ? 3400 : 3398;
					else
						iMessageLocale = bBarnDetail ? 3397 : 3399;

					AMiscFunctions::ShowMessageBox( 1066, iMessageLocale );
					break;
				}
				case 5:
					AMiniGameManager::GetSingleton()->RequestReturn();
					break;
				default:
					break;
			}

			m_iGameOverStep++;
		}
	}
	else
	{
		if ( m_oStartHudElement.IsForceHidden() == TTRUE )
		{
			m_oLevelPopup.ShowIfHasText();
			m_aBonusPopups[ 0 ].ShowIfHasText();
			m_aBonusPopups[ 1 ].ShowIfHasText();
			m_aBonusPopups[ 2 ].ShowIfHasText();
			m_oEndPopup.ShowIfHasText();
			m_oTurnEndPopup.ShowIfHasText();
		}

		UpdateRestart();
		AMiniGameState::OnActivate();

		if ( m_bWasVisible == TTRUE )
		{
			m_GUIElement.Show();
			return;
		}
	}

	m_GUIElement.Hide();
}

// $Barnyard: FUNCTION 004a84f0
void ACowTippingMiniGameState::OnDeactivate()
{
	AInputHandler::GetSingleton()->SetRumblePaused( TTRUE );

	m_oLevelPopup.Hide();
	m_aBonusPopups[ 0 ].Hide();
	m_aBonusPopups[ 1 ].Hide();
	m_aBonusPopups[ 2 ].Hide();
	m_oEndPopup.Hide();
	m_oTurnEndPopup.Hide();

	AMusicManager::GetSingleton()->SetVolume( m_fSavedMusicVolume );

	m_bWasVisible = m_GUIElement.IsVisible();
	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 004a7be0
void ACowTippingMiniGameState::OnObjectCollected( AAnimatableObject* a_pObject )
{
	Coin* pEntry = TNULL;

	for ( TINT i = 0; i < 8; i++ )
	{
		if ( CT_GetCoin( this, i )->pObject == a_pObject )
		{
			pEntry = CT_GetCoin( this, i );
			break;
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail dereferences TNULL if the object isn't one of the entries
	if ( !pEntry )
		return;
#endif

	pEntry->pObject->SetSkeletonUpdating( TFALSE, TTRUE );
	pEntry->iState = Coin::COINSTATE_FREE;
}

//-----------------------------------------------------------------------------
// AMiniGameState
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004ad090
void ACowTippingMiniGameState::Pause()
{
	SetTimeOfDay( 0.0f );

	if ( m_aStages[ m_iStage ].iCameraSettings != 1 && m_aStages[ m_iStage ].iStageType != Stage::STAGETYPE_RACCOONS )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "ssct_intro", "ssct_intro", "SSct_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
}

// Lost
// $Barnyard: FUNCTION 004aae90
void ACowTippingMiniGameState::PlayLoseSequence()
{
	if ( m_aStages[ m_iStage ].iCameraSettings != 1 )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "ssct_lose", "ssct_lose", "SSct_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
}

// Won
// $Barnyard: FUNCTION 004bccd0
void ACowTippingMiniGameState::PlayWinSequence()
{
	if ( m_aStages[ m_iStage ].iCameraSettings == 1 )
	{
		// The story version: cleans up and plays the SS10 ending
		Cleanup();
		m_bStoryEnding = TTRUE;

		AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
		AMusicManager::GetSingleton()->SetForcedSound( ASOUNDID_INVALID );
		AMusicManager::GetSingleton()->ForceUpdate();

		AGameTimeFXManager::GetSingleton()->SetDayPhase( AGameTimeManager::DAYPHASE_NIGHT, TFALSE, 0.0f );
		AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );

		ASSeqState* pSSeqState = new ASSeqState( "SS10_end", "SS10", "SS10_cameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE );
		pSSeqState->SetLoading( TTRUE );
		AGameStateController::GetSingleton()->PushState( pSSeqState );

		AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
	}
	else
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "ssct_win", "ssct_win", "SSct_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
}

// $Barnyard: FUNCTION 004a7af0
void ACowTippingMiniGameState::OnPreStart()
{
	if ( m_aStages[ m_iStage ].iCameraSettings != 1 && m_aStages[ m_iStage ].iStageType != Stage::STAGETYPE_RACCOONS )
		m_pLauncher->SetSkeletonUpdating( TFALSE, TTRUE );
}

// $Barnyard: FUNCTION 004a7500
TBOOL ACowTippingMiniGameState::IsGameOver()
{
	return m_bGameOver;
}

// $Barnyard: FUNCTION 004a74d0
void ACowTippingMiniGameState::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	if ( m_bGameOver )
	{
		if ( m_iGameOverStep == 0 )
			PlayWinSequence();

		m_iGameOverStep++;
	}
}

// $Barnyard: FUNCTION 004c1a60
void ACowTippingMiniGameState::UpdatePreStart( TFLOAT a_fDeltaTime )
{
	UpdateReload();
	UpdateCamera( a_fDeltaTime );
	UpdatePlayerControls( a_fDeltaTime );

	m_fInputX      = 0.0f;
	m_fMoveY       = 0.0f;
	m_bNoInputX    = TFALSE;
	m_bNoInputY    = TFALSE;
	m_fAccelTimerX = TMath::MAXFLOAT;
	m_fAccelTimerY = TMath::MAXFLOAT;
	m_bUnk_13404   = TTRUE;
	m_pLockUnit    = 0;

	m_oTargetReticle.SetActive( TFALSE, TTRUE, 50.0f );
}

// $Barnyard: FUNCTION 004ac2e0
AMiniGameState::S1* ACowTippingMiniGameState::GetResults()
{
	TINT iWinner    = -1;
	TINT iBestScore = 0;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		TINT        iScore = m_aPlayers[ i ].iScore;
		PlayerInfo& rInfo  = m_oResults.oPlayerList[ i ];

		rInfo.iUnk2 = iScore;
		rInfo.bUnk1 = TTRUE;

		if ( iScore > iBestScore )
		{
			iWinner    = i;
			iBestScore = iScore;
		}
	}

	if ( m_iNumPlayers == 1 )
	{
		PlayerData& rPlayer = m_aPlayers[ 0 ];

		if ( rPlayer.iLives <= 0 || rPlayer.iLives2 <= 0 || ( rPlayer.iLevel < m_iNumLevels && rPlayer.iLevel < m_aStages[ m_iStage ].iNumLevels ) )
		{
			m_oResults.iResult = -1;
			iWinner            = m_oResults.iResult;
		}
		else
		{
			m_oResults.iResult = iWinner;
		}
	}
	else
	{
		m_oResults.iResult = iWinner;
	}

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.bWon           = iWinner != -1;

	PlayerInfo& rInfo = m_oResults.oPlayerList[ m_iCurrentPlayer ];

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( CT_IsMode23( iMode ) )
	{
		m_oResults.bFlag6 = TTRUE;
		m_oResults.bFlag4 = TFALSE;
		m_oResults.bFlag5 = TTRUE;
	}
	else
	{
		TINT iCows = m_iEnemyScore + m_iRaccoonScore + m_iBonusScore;
		rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( ( iMode == Stage::STAGETYPE_RACCOONS ) ? 579 : 580 ), iCows } );
		rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( 578 ), m_iCoyoteScore } );

		if ( m_aStages[ m_iStage ].iStageType == Stage::STAGETYPE_RACCOONS )
			rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( 584 ), 20 * m_aPlayers[ m_iCurrentPlayer ].iLives } );

		rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( 585 ), 20 * m_aPlayers[ m_iCurrentPlayer ].iLives2 } );

		m_oResults.ComputeTotal( m_iCurrentPlayer );

		if ( m_bSubmitHighScore )
		{
			m_bSubmitHighScore = TFALSE;

			TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

			if ( TUINT( iVariant ) <= 2 )
			{
				// NOTE: the score of the first player is submitted whoever played
				AHighScoreManager::AddScore( ( iVariant == 2 ) ? 45 : 44, TFLOAT( m_oResults.oPlayerList[ 0 ].iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			}
		}
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 004aac90
void ACowTippingMiniGameState::OnStartHudFinished()
{
	AMiniGameState::OnStartHudFinished();

	m_oTargetReticle.SetActive( TTRUE, TTRUE, 50.0f );
	m_oFloatHudElement.Show();

	m_oLevelPopup.ShowIfHasText();
	m_aBonusPopups[ 0 ].ShowIfHasText();
	m_aBonusPopups[ 1 ].ShowIfHasText();
	m_aBonusPopups[ 2 ].ShowIfHasText();
	m_oEndPopup.ShowIfHasText();
	m_oTurnEndPopup.ShowIfHasText();

	if ( !CT_IsMode23( m_aStages[ m_iStage ].iStageType ) )
		ShowLevelPopups( 0 );

	ASoundManager::GetSingleton()->PlayCue( soundbank::DAGBOSS_OTIS_RESCUE1 );
}

// Fades the music in while the timer at 0x1334c is running and out once it has run out.
// $Barnyard: FUNCTION 004aadd0
void ACowTippingMiniGameState::UpdateMusicVolume( TFLOAT a_fDeltaTime )
{
	m_fMusicTimer -= a_fDeltaTime;

	AMusicManager* pMusicManager = AMusicManager::GetSingleton();
	TFLOAT         fVolume       = pMusicManager->GetVolume();

	if ( m_fMusicTimer > 0.0f )
	{
		if ( fVolume >= 1.0f )
			return;

		fVolume = a_fDeltaTime * m_fMusicFadeInSpeed + fVolume;
	}
	else
	{
		if ( fVolume <= 0.0f )
			return;

		fVolume = fVolume - a_fDeltaTime * m_fMusicFadeOutSpeed;
	}

	if ( fVolume < 0.0f )
		fVolume = 0.0f;

	pMusicManager->SetVolume( fVolume );
}

//-----------------------------------------------------------------------------
// Own methods
//-----------------------------------------------------------------------------

// Number of the used Crates of a_iGroup
// $Barnyard: FUNCTION 004a7020
TINT ACowTippingMiniGameState::GetNumUsedCrates( TINT a_iGroup )
{
	TINT iCount = 0;

	for ( TINT i = 4; i >= 0; i-- )
	{
		if ( CT_GetCrate( this, a_iGroup, i )->iState != Crate::CRATESTATE_FREE )
			iCount++;
	}

	return iCount;
}

// Adds the kinematics obstruction of the unit to the network
// $Barnyard: FUNCTION 004a7160
void ACowTippingMiniGameState::AddUnitObstruction( Enemy* a_pUnk )
{
	AInteriorNetwork*       pNetwork     = m_pNetwork;
	AKinematicsObstruction* pObstruction = TSTATICCAST( AKinematicsObstruction, TREINTERPRETCAST( AUnit*, a_pUnk->pUnit )->GetObstruction() );

	// NOTE: unlike AInteriorNetwork::AddUnitObstruction (00551510) this doesn't check for TNULL
	if ( pNetwork->m_vecObstructions.Find( pObstruction ) == pNetwork->m_vecObstructions.End() )
		pNetwork->m_vecObstructions.PushBack( pObstruction );
}

// $Barnyard: FUNCTION 004a8360
void ACowTippingMiniGameState::DestroyCows()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < m_iNumCows; i++ )
	{
		m_aCows[ i ].oSkinSwap.Reset();
		pManager->DestroyAnimatableObject( m_aCows[ i ].pObject );
	}
}

// $Barnyard: FUNCTION 004a83b0
void ACowTippingMiniGameState::DestroyProjectiles()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	// NOTE: the result isn't used
	pManager->FindType( TPS8( cowtipping_tomato ) );

	for ( TINT i = 0; i < 2; i++ )
	{
		m_aProjectileLists[ i ].llFree.Clear();
		m_aProjectileLists[ i ].llHeld.Clear();
	}

	for ( TINT i = MAX_PROJECTILES - 1; i >= 0; i-- )
		pManager->DestroyAnimatableObject( m_aProjectiles[ i ].pObject );
}

// $Barnyard: FUNCTION 004a8410
void ACowTippingMiniGameState::DestroyCrates()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < 6; i++ )
	{
		for ( TINT k = 4; k >= 0; k-- )
			pManager->DestroyAnimatableObject( CT_GetCrate( this, i, k )->pObject );
	}
}

// $Barnyard: FUNCTION 004a8460
void ACowTippingMiniGameState::DestroyEnemies()
{
	CT_SList( m_llActiveEnemies ).Clear();
	CT_SList( m_llDoneEnemies ).Clear();

	for ( TINT i = TARRAYSIZE( m_aEnemies ) - 1; i >= 0; i-- )
	{
		if ( m_aEnemies[ i ].pUnit != 0 )
		{
			m_aEnemies[ i ].oSkinSwap.Reset();
			AUnitManager::GetSingleton()->DestroyUnit( TREINTERPRETCAST( AUnit*, m_aEnemies[ i ].pUnit ) );
		}
	}
}

// $Barnyard: FUNCTION 004a84b0
void ACowTippingMiniGameState::DestroyGophers()
{
	CT_SList( m_llActiveGophers ).Clear();
	CT_SList( m_llFreeGophers ).Clear();

	for ( TINT i = TARRAYSIZE( m_aGophers ) - 1; i >= 0; i-- )
	{
		if ( m_aGophers[ i ].pUnit != 0 )
			AUnitManager::GetSingleton()->DestroyUnit( TREINTERPRETCAST( AUnit*, m_aGophers[ i ].pUnit ) );
	}
}

// Random group (0-5), groups 3 and 5 aren't used in mode 0
// $Barnyard: FUNCTION 004a8620
TINT ACowTippingMiniGameState::GetRandomCrateGroup()
{
	TINT iGroup = ARandom::GetSingleton()->m_oRandom.GetInt( 6 );

	if ( m_aStages[ m_iStage ].iStageType == Stage::STAGETYPE_KIDS )
	{
		while ( iGroup == 3 || iGroup == 5 )
			iGroup = ARandom::GetSingleton()->m_oRandom.GetInt( 6 );
	}

	return iGroup;
}

// $Barnyard: FUNCTION 004a9ac0
void ACowTippingMiniGameState::SelectPrevious( TINT a_iPlayer )
{
	if ( m_iNumHumanPlayers > 1 )
	{
		TINT& rValue = m_aPlayers[ a_iPlayer ].iSelectedTipSpot;

		if ( --rValue < 0 )
			rValue = m_iNumTipSpots - 1;
	}
}

// $Barnyard: FUNCTION 004a9b00
void ACowTippingMiniGameState::SelectNext( TINT a_iPlayer )
{
	if ( m_iNumHumanPlayers > 1 )
	{
		TINT& rValue = m_aPlayers[ a_iPlayer ].iSelectedTipSpot;

		if ( ++rValue >= m_iNumTipSpots )
			rValue = 0;
	}
}

// $Barnyard: FUNCTION 004ab030
void ACowTippingMiniGameState::DestroyCoins()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 7; i >= 0; i-- )
		pManager->DestroyAnimatableObject( CT_GetCoin( this, i )->pObject );
}

// The last Enemy of list 0x13124 of type a_iType
// $Barnyard: FUNCTION 004ab4b0
ACowTippingMiniGameState::Enemy* ACowTippingMiniGameState::FindFreeEnemyOfType( TINT a_iType )
{
	T2GenericSList& rList  = CT_SList( m_llDoneEnemies );
	Enemy*          pFound = TNULL;

	for ( T2GenericSList::Node* it = rList.Begin(); it != rList.End(); it = it->Next() )
	{
		if ( CT_Node<Enemy>( it )->eType == a_iType )
			pFound = CT_Node<Enemy>( it );
	}

	return pFound;
}

// A random Enemy of list 0x13124
// $Barnyard: FUNCTION 004ab500
ACowTippingMiniGameState::Enemy* ACowTippingMiniGameState::GetRandomFreeEnemy()
{
	T2GenericSList& rList   = CT_SList( m_llDoneEnemies );
	TINT            iPick   = 0;
	Enemy*          pPicked = TNULL;

	if ( TINT( rList.Size() ) > 1 )
		iPick = ARandom::GetSingleton()->m_oRandom.GetInt( rList.Size() - 1 );

	TINT i = 0;

	for ( T2GenericSList::Node* it = rList.Begin(); it != rList.End(); it = it->Next(), i++ )
	{
		if ( i == iPick )
			pPicked = CT_Node<Enemy>( it );
	}

	return pPicked;
}

// $Barnyard: FUNCTION 004ab7e0
TINT ACowTippingMiniGameState::GetNumActiveEnemies()
{
	T2GenericSList& rList  = CT_SList( m_llActiveEnemies );
	TINT            iCount = rList.Size();

	if ( m_aEnemies[ 12 ].eState != Enemy::ENEMYSTATE_INACTIVE )
		iCount--;

	for ( T2GenericSList::Node* it = rList.Begin(); it != rList.End(); it = it->Next() )
	{
		if ( CT_Node<Enemy>( it )->eState == Enemy::ENEMYSTATE_RETURNING )
			iCount--;
	}

	return iCount;
}

// $Barnyard: FUNCTION 004ab840
TINT ACowTippingMiniGameState::GetNumActiveEnemiesOfType( TINT a_iType )
{
	T2GenericSList& rList  = CT_SList( m_llActiveEnemies );
	TINT            iCount = 0;

	for ( T2GenericSList::Node* it = rList.Begin(); it != rList.End(); it = it->Next() )
	{
		Enemy* pUnk = CT_Node<Enemy>( it );

		if ( pUnk->eType == a_iType && pUnk->eState != Enemy::ENEMYSTATE_RETURNING )
			iCount++;
	}

	return iCount;
}

// $Barnyard: FUNCTION 004ae650
void ACowTippingMiniGameState::DestroyRaccoons()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	CT_SList( m_llActiveRaccoons ).Clear();
	CT_SList( m_llFreeRaccoons ).Clear();

	for ( TINT i = TARRAYSIZE( m_aRaccoons ) - 1; i >= 0; i-- )
	{
		AAnimatedUnit* pUnit = m_aRaccoons[ i ].pUnit;

		if ( pUnit )
		{
			DropKidTomato( TINT( &m_aRaccoons[ i ] ) );

			AAnimatableObject* pObject;

			if ( CT_IsMode23( m_aStages[ m_iStage ].iStageType ) )
				pObject = pUnit->GetAnimatableObject();
			else
				pObject = pUnit->GetAnimatableObject()->FindAttachedObject( m_iRaccoonHandBone );

			if ( pObject != pUnit->GetAnimatableObject() )
			{
				pManager->DetachObject( pObject, pUnit->GetAnimatableObject() );
				pManager->DestroyAnimatableObject( pObject );
			}

			AUnitManager::GetSingleton()->DestroyUnit( pUnit );
		}
	}
}

// Takes a free Crate of a_iGroup for the Raccoon a_iIndex
// $Barnyard: FUNCTION 004ae820
ACowTippingMiniGameState::Crate* ACowTippingMiniGameState::TakeCrate( TINT a_iIndex, TINT a_iGroup )
{
	if ( m_aStages[ m_iStage ].iStageType == Stage::STAGETYPE_KIDS && ( a_iGroup == 3 || a_iGroup == 5 ) )
		a_iGroup = GetRandomCrateGroup();

	TINT   iSlot = 4;
	Crate* pUnk  = CT_GetCrate( this, a_iGroup, iSlot );

	while ( pUnk->iState != Crate::CRATESTATE_FREE )
	{
		if ( --iSlot < 0 )
			return TNULL;

		pUnk--;
	}

	pUnk->iState = Crate::CRATESTATE_CARRIED;
	pUnk->pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	pUnk->pObject->GetModelInstance()->SetOpacity( 1.0f );
	pUnk->pObject->SetVisible( TFALSE );

	pUnk->iBit  = a_iIndex;
	pUnk->iHits = 0;
	CT_SetBit( m_auiPickupBits, a_iIndex );
	pUnk->fTimeLeft     = m_fBounceTime;
	pUnk->fBlinkPhase   = 0;
	pUnk->vDirection    = TVector4( -1.0f, 0.0f, 0.0f, 1.0f );
	pUnk->fTurnProgress = 0;

	pUnk->pObject->GetModelInstance()->GetPosition( pUnk->vPosition );
	pUnk->vPosition.y = 0.0f;

	pUnk->pObject->SetAnimControllersDisabled( TFALSE );

	m_oRunnerIcon.SetShown( TTRUE );

	return pUnk;
}

// Goal a_szGoalName (index a_iIndex) of the network at 0x57d0
// $Barnyard: FUNCTION 004b0a70
void* ACowTippingMiniGameState::FindGoal( TINT a_iIndex, const TCHAR* a_szGoalName )
{
	return m_pNetwork->FindGoalByIndex( TPString8( a_szGoalName ), a_iIndex );
}

// $Barnyard: FUNCTION 004b53f0
TBOOL ACowTippingMiniGameState::PlaceWildMikeBox( TINT a_iIndex )
{
	AAnimatableObject* pObject = m_pWildMikeBox;

	if ( CT_IsMode23( m_aStages[ m_iStage ].iStageType ) )
		pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	HideRunner();
	m_fRunnerPipeTimer = m_fRunnerPipeTime;

	// NOTE: the positions (0x11dc) and directions (0x125c) are 8 TVector4s each
	pObject->GetModelInstance()->SetTranslation( m_aWildMikeGoalPositions[ a_iIndex ].AsVector3() );
	pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	pObject->GetModelInstance()->SetDirection( m_aWildMikeGoalDirections[ a_iIndex ] );
	pObject->KillAllAnimations();
	pObject->PlayAnimation( TPS8( up ) );

	m_eRunnerState = RUNNERSTATE_PIPE_UP;

	AAnimatableObject* pObject2 = m_pHanna;

	if ( !pObject2->IsAnimationPlanned( TPS8( distress_pleased ) ) && !pObject2->IsAnimPlaying( TPS8( distress_pleased ), 0.0f ) )
		pObject2->FinishAnimation( TPS8( distress_pleased ) );

	pObject2->PlayAnimation( TPS8( distress_pleased ) );

	m_oRunnerIcon.SetShown( TTRUE );
	return TTRUE;
}

// Takes a free Coin (0x13128) and places it at a_pPosition
// $Barnyard: FUNCTION 004b57d0
ACowTippingMiniGameState::Coin* ACowTippingMiniGameState::TakeCoin( const Toshi::TVector4* a_pPosition )
{
	TINT  iIndex = 0;
	Coin* pEntry = CT_GetCoin( this, 0 );

	while ( pEntry->iState != Coin::COINSTATE_FREE )
	{
		if ( ++iIndex >= 8 )
			return TNULL;

		pEntry++;
	}

	pEntry->pObject->SetOpacity( 1.0f, TTRUE );
	pEntry->pObject->SetSkeletonUpdating( TTRUE, TTRUE );

	TMatrix44 oMatrix;

	// NOTE: retail gets the local transform and overwrites it right away
	pEntry->pObject->GetModelInstance()->GetLocalTransform( oMatrix );
	oMatrix                  = TMatrix44::IDENTITY;
	oMatrix.GetTranslation() = TVector4( a_pPosition->x, a_pPosition->y + 2.0f, a_pPosition->z, a_pPosition->w );
	oMatrix.RotateX( TMath::PI * 0.5f );
	pEntry->pObject->GetModelInstance()->SetTransform( oMatrix );

	pEntry->fTimeLeft = m_fBounceTime;
	pEntry->iUnk00    = 0;
	pEntry->fVelocity = -5.0f;
	pEntry->fFallTime = 0;
	pEntry->vPosition = *a_pPosition;
	pEntry->iState    = Coin::COINSTATE_BOUNCING;

	return pEntry;
}

// Cleans up everything the state has created (called by OnRemoval and when the story version is won)
// $Barnyard: FUNCTION 004b8370
void ACowTippingMiniGameState::Cleanup()
{
	m_aProjectiles[ 0 ].pObject->SetGlow( TFALSE );
	AProjectile::ResetTrajectoryGravity();

	TFree( TREINTERPRETCAST( void*, m_ppCowSpinSprites ) );
	TFree( TREINTERPRETCAST( void*, m_ppCowFallSprites ) );
	TFree( TREINTERPRETCAST( void*, m_ppCowShockSprites ) );
	TFree( TREINTERPRETCAST( void*, m_ppCowSleepSprites ) );
	TFree( TREINTERPRETCAST( void*, m_ppCowShockedSprites ) );

	ARenderer::GetSingleton()->SetFar( m_fSavedFarDistance );

	if ( m_aStages[ m_iStage ].iCameraSettings == 0 )
	{
		ASimAnimModelHelperManager::GetSingleton()->RemoveModelHelper( &m_oWindmillHelper );
		m_oWindmillHelper.OnDestroy();
	}

	APositionalSoundSet::GetSingleton()->SetPaused( TFALSE );
	AInputHandler::GetSingleton()->StopAllRumbles();
	// DOES_NOTHING (00628f40) is called on AInputHandler with TFALSE
	AInputHandler::GetSingleton()->StopRumble( AInputHandler::GetSingleton()->GetMainController() );

	// The list at 0x13278 (T2GenericDList) is cut off from its items
	T2GenericDList::Node* pRoot = TREINTERPRETCAST( T2GenericDList::Node*, &m_oAnimEventListener );

	if ( pRoot->Next() != pRoot )
		pRoot->Remove();

	m_oAnimEventListener.pCaller   = TNULL;
	m_oAnimEventListener.pCallback = TNULL;

	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pLauncher );

	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( CT_IsMode23( iMode ) )
		m_oButtonSlamHud.Detach();
	else
		DetachGridObjects();

	DestroyCows();
	DestroyProjectiles();
	DestroyEnemies();
	DestroyRaccoons();

	if ( CT_IsMode23( iMode ) )
	{
		if ( CT_IsMode23( m_aStages[ m_iStage ].iStageType ) )
		{
			AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pWildMikeBox );
			AUnitManager::GetSingleton()->DestroyUnit( TREINTERPRETCAST( AUnit*, m_pWildMike ) );
		}

		if ( CT_IsMode23( m_aStages[ m_iStage ].iStageType ) )
		{
			AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pHanna );
			AUnitManager::GetSingleton()->DestroyUnit( TREINTERPRETCAST( AUnit*, m_pOtis ) );
		}
	}
	else
	{
		DestroyGophers();
		DestroyCrates();
		DestroyCoins();
	}

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( 1.0471976f );
	AGameTimeFXManager::GetSingleton()->ClearForcedFX();
	ATreeManager::GetSingleton()->SetDistances( m_fSavedTreeLODDistance, m_fSavedTreeRenderDistance, m_fSavedTreeBlendingDistance );

	m_oLevelPopup.Hide();
	m_aBonusPopups[ 0 ].Hide();
	m_aBonusPopups[ 1 ].Hide();
	m_aBonusPopups[ 2 ].Hide();
	m_oEndPopup.Hide();
	m_oTurnEndPopup.Hide();

	ASimpleProjectileManager::GetSingleton()->DestroyAllPools();

	if ( AMilkManager::GetSingleton() )
		delete AMilkManager::GetSingleton();

	m_oCowShockMarks.RemoveSelf();
	m_aLivesTexts[ 1 ].RemoveSelf();
	m_aLivesTexts[ 0 ].RemoveSelf();
	m_oDagText.RemoveSelf();
	m_oDagMug.RemoveSelf();
	m_oButtonSlamHud.m_oBar.Hide();
	m_oElementPopup.RemoveSelf();
	m_oRunnerIcon.RemoveSelf();
	m_oRaccoonWarning.RemoveSelf();
	m_oLives2Hud.RemoveSelf();
	m_oLives1Hud.RemoveSelf();

	for ( TINT i = 2; i >= 0; i-- )
		m_aFlarePopups[ i ].RemoveSelf();

	m_oFloatHudElement.RemoveSelf();
	m_oPortrait.Destroy();
}

// A random Raccoon index (with the variant in a_pOutVariant if the mode isn't 2/3), -1 if there's none
// $Barnyard: FUNCTION 004b9010
TINT ACowTippingMiniGameState::GetRandomStartIndex( TBOOL* a_pOutVariant )
{
	CT_RandomPicker<30> oPicker;
	TINT                iNumFound = 0;

	if ( m_iNumTipSpots <= 0 )
		return -1;

	for ( TINT i = 0; i < m_iNumTipSpots; i++ )
	{
		if ( !CT_IsBitSet( m_auiCowSideReservedBits, i ) && !CT_IsBitSet( m_auiCowSideTakenBits, i ) && TUINT8( m_aCows[ i ].bActive ) == 1 )
		{
			oPicker.Add( i );
			iNumFound++;
		}
	}

	if ( iNumFound <= 0 )
		return -1;

	TINT iResult = oPicker.Pick();

	if ( !CT_IsMode23( m_aStages[ m_iStage ].iStageType ) )
	{
		*a_pOutVariant = iResult & 1;
		iResult /= 2;
	}

	return iResult;
}

// A random Raccoon index other than the last one (0x14d50), -1 if there's none
// $Barnyard: FUNCTION 004b92a0
TINT ACowTippingMiniGameState::GetRandomRaccoonIndex()
{
	CT_RandomPicker<10> oPicker;
	TINT                iNumFound = 0;

	for ( TINT i = 0; i < m_iNumRaccoons; i++ )
	{
		if ( i != m_iLastRaccoonSpot && !CT_IsBitSet( m_auiUsedRaccoonBits, i ) )
		{
			oPicker.Add( i );
			iNumFound++;
		}
	}

	if ( iNumFound <= 0 )
	{
		if ( CT_IsBitSet( m_auiUsedRaccoonBits, m_iLastRaccoonSpot ) )
			return -1;

		return m_iLastRaccoonSpot;
	}

	m_iLastRaccoonSpot = oPicker.Pick();
	return m_iLastRaccoonSpot;
}

// A random free index (bits at 0x57b0), -1 if there's none
// $Barnyard: FUNCTION 004b9370
TINT ACowTippingMiniGameState::GetRandomGopherIndex()
{
	CT_RandomPicker<14> oPicker;
	TINT                iNumFound = 0;

	if ( m_iNumGophers <= 0 )
		return -1;

	for ( TINT i = 0; i < m_iNumGophers; i++ )
	{
		if ( !CT_IsBitSet( m_auiPickupBits, i ) )
		{
			oPicker.Add( i );
			iNumFound++;
		}
	}

	if ( iNumFound <= 0 )
		return -1;

	return oPicker.Pick();
}

// Activates the Raccoon a_iIndex of list 0x1326c (moves it to 0x13268)
// $Barnyard: FUNCTION 004b9780
TBOOL ACowTippingMiniGameState::ActivateRaccoon( TINT a_iIndex, TINT a_iType )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return TFALSE;

	T2GenericSList& rList = CT_SList( m_llFreeRaccoons );
	Raccoon*        pUnk  = TNULL;

	for ( T2GenericSList::Node* it = rList.Begin(); it != rList.End(); it = it->Next() )
	{
		if ( CT_Node<Raccoon>( it )->iSpot == a_iIndex )
			pUnk = CT_Node<Raccoon>( it );
	}

	if ( !pUnk )
		return TFALSE;

	AAnimatedUnit* pUnit  = pUnk->pUnit;
	TINT           iMode  = m_aStages[ m_iStage ].iStageType;
	Level&         rLevel = m_aLevels[ m_aPlayers[ m_iCurrentPlayer ].iLevel ];

	pUnk->iType = a_iType;

	if ( CT_IsMode23( iMode ) )
	{
		pUnk->fDrawTimer = rLevel.fRaccoonDrawTimeLarge;
		pUnk->fDownTimer = rLevel.fRaccoonDownTimeLarge;

		if ( a_iType == 1 || a_iType == 2 )
		{
			DropKidTomato( TINT( pUnk ) );
			pUnit->GetAnimatableObject()->PlayAnimation( TPS8( uptawnt ) );
		}
		else
		{
			LoadKidSlingshot( TREINTERPRETCAST( void**, pUnk ) );
			// NOTE: the last two arguments are 1.0f in retail
			m_oButtonSlamHud.StartFade( 0.0f, 1.0f, 1.0f );
			pUnit->GetAnimatableObject()->PlayAnimation( TPS8( up ) );
		}

		m_fRunnerPipeTimer = 0;
	}
	else
	{
		pUnk->fDrawTimer = rLevel.fRaccoonDrawTime;
		pUnk->fDownTimer = rLevel.fRaccoonDownTime;
		pUnit->GetAnimatableObject()->PlayAnimation( TPS8( up ) );
	}

	pUnk->iHits = 0;
	rList.Transfer( CT_AsNode( pUnk ), CT_SList( m_llActiveRaccoons ) );

	m_fRaccoonShootTimer = m_fRaccoonShootDelay + m_fRaccoonShootTimer;
	CT_SetBit( m_auiUsedRaccoonBits, pUnk->iSpot );

	pUnk->iState = PopUpUnit::POPUPSTATE_ACTIVE;
	pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );
	*TREINTERPRETCAST( TUINT8*, &pUnk->bTargeted ) = 0;
	pUnk->iShots                                   = 0;

	TurnUnitTowardsPlayer( TINT( pUnit ) );

	if ( !CT_IsMode23( m_aStages[ m_iStage ].iStageType ) )
		m_oRaccoonWarning.SetShown( TTRUE );

	pUnit->SetVisible( TTRUE );
	m_fMusicTimer = m_fMusicDelay;

	return TTRUE;
}

// Activates a random Gopher-type unit of list 0x13274 at a random free goal and gives it an Crate of a_iGroup
// $Barnyard: FUNCTION 004b99a0
TBOOL ACowTippingMiniGameState::ActivateGopher( TINT a_iGroup )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return TFALSE;

	T2GenericSList& rList = CT_SList( m_llFreeGophers );

	if ( rList.Begin() == rList.End() )
		return TFALSE;

	TINT iIndex = GetRandomGopherIndex();

	if ( iIndex <= -1 )
		return TFALSE;

	TINT iPick = 0;

	if ( TINT( rList.Size() ) > 1 )
		iPick = ARandom::GetSingleton()->m_oRandom.GetInt( rList.Size() - 1 );

	Raccoon* pUnk = TNULL;
	TINT     i    = 0;

	for ( T2GenericSList::Node* it = rList.Begin(); it != rList.End(); it = it->Next(), i++ )
	{
		// NOTE: retail always takes the first item instead of it
		if ( i == iPick )
			pUnk = CT_Node<Raccoon>( rList.Begin() );
	}

	AAnimatedUnit* pUnit = pUnk->pUnit;
	pUnit->SetVisible( TTRUE );
	pUnk->iSpot = iIndex;
	pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );

	ASteer*        pSteer = pUnit->GetSteer();
	AInteriorGoal* pGoal  = TSTATICCAST( AInteriorGoal, FindGoal( iIndex, "gopher" ) );

	TVector4 vPosition;
	pGoal->GetPosition( vPosition );
	pSteer->SetPosition( vPosition );

	TVector4 vDirection(
	    m_vCameraBasePos.x - vPosition.x,
	    m_vCameraBasePos.y - vPosition.y,
	    m_vCameraBasePos.z - vPosition.z,
	    CT_FLOAT( m_vCameraBasePos.w )
	);
	vDirection.Normalise();
	pSteer->SetDirection( vDirection );

	pUnit->SetHashed( TTRUE );
	pUnit->SetNoQuery( TFALSE );
	pUnk->iHits = 0;
	pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

	rList.Transfer( CT_AsNode( pUnk ), CT_SList( m_llActiveGophers ) );
	m_fGopherTimer = 5.0f;

	pUnit->GetAnimatableObject()->PlayAnimation( TPS8( up ) );
	pUnk->iState = PopUpUnit::POPUPSTATE_ACTIVE;

	AAnimatableObjectManager* pManager  = AAnimatableObjectManager::GetSingleton();
	AAnimatableObject*        pAttached = pManager->GetAttachedObjectByBoneName( pUnit->GetAnimatableObject(), TPS8( cb_rlocator ), 0 );

	if ( pAttached )
		HidePickup( FindCrateByObject( TINT( pAttached ) ) );

	Crate* pCrate = TakeCrate( iIndex, a_iGroup );

	if ( pCrate )
	{
		pManager->AttachObject( pCrate->pObject, pUnit->GetAnimatableObject(), TPString8(), TPS8( cb_rlocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
	}

	return TTRUE;
}

// A random free index of the positions at 0x12dc (bits at 0x57bc), -1 if there's none
// $Barnyard: FUNCTION 004bce40
TINT ACowTippingMiniGameState::GetRandomCoinIndex()
{
	CT_RandomPicker<8> oPicker;
	TINT               iNumFound = 0;

	if ( m_iNumCoins <= 0 )
		return -1;

	for ( TINT i = 0; i < m_iNumCoins; i++ )
	{
		if ( !CT_IsBitSet( m_auiUsedCoinBits, i ) )
		{
			oPicker.Add( i );
			iNumFound++;
		}
	}

	if ( iNumFound <= 0 )
		return -1;

	return oPicker.Pick();
}

// Spawns an Enemy of a_iType (list 0x13124 -> 0x13120) at the goal "start<a_iIndex + 1><a|b>"
// $Barnyard: FUNCTION 004c2b20
TBOOL ACowTippingMiniGameState::SpawnEnemy( TINT a_iType, TINT a_iIndex, TBOOL a_bVariant, TBOOL a_bUnk )
{
	if ( a_iIndex == -1 )
		return TFALSE;

	if ( m_eTurnEnd != TURNEND_NONE )
		return TFALSE;

	Enemy* pUnk = FindFreeEnemyOfType( a_iType );

	if ( !pUnk )
		return TFALSE;

	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( CT_IsMode23( iMode ) )
		a_bVariant = TFALSE;

	TINT iBit = CT_IsMode23( iMode ) ? a_iIndex : a_bVariant + a_iIndex * 2;

	if ( CT_IsBitSet( m_auiCowSideReservedBits, iBit ) && ( CT_IsBitSet( m_auiCowSideTakenBits, iBit ) || TUINT8( m_aCows[ iBit ].bActive ) != 1 ) )
	{
		a_iIndex = GetRandomStartIndex( &a_bVariant );

		if ( a_iIndex < 0 )
			return TFALSE;
	}

	AAnimatedUnit* pUnit = pUnk->pUnit;
	pUnit->SetVisible( TTRUE );
	AddUnitObstruction( pUnk );

	pUnk->iCowSide = a_bVariant;
	pUnk->iCow     = a_iIndex;

	if ( iMode == Stage::STAGETYPE_KIDS )
		pUnk->oSkinSwap.SetObject( TREINTERPRETCAST( AAnimatableObject*, pUnit->GetAnimatableObject() ), m_aiSkinIndex[ a_iType ] );

	pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

	// Cycles through the skins of the type
	if ( ++m_aiSkinIndex[ a_iType ] >= pUnit->GetAnimatableObject()->GetObjectType()->GetNumSkins() )
		m_aiSkinIndex[ a_iType ] = 0;

	pUnk->bUnk4E        = TFALSE;
	pUnk->iNumTargeting = 0;
	pUnit->ChangeSteer( &TGetClass( AInteriorSteer ) );

	TString8 strGoal;

	if ( CT_IsMode23( iMode ) )
		strGoal.Format( "%s%da", "start", pUnk->iCow );
	else
		strGoal.Format( "%s%d%c", "start", pUnk->iCow + 1, 'a' + ( pUnk->iCowSide != 0 ) );

	TPString8 strGoalName( strGoal.GetString() );

	ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, pUnit->GetSteer() );
	pSteer->SetInteracting( 0 );
	pSteer->SetNetwork( m_pNetwork, 0 );
	pSteer->Command_SnapToGoal( strGoalName );

	pUnit->SetHashed( TTRUE );
	pUnit->SetNoQuery( TFALSE );
	pUnk->iHits = 0;
	CT_SList( m_llDoneEnemies ).Transfer( CT_AsNode( pUnk ), CT_SList( m_llActiveEnemies ) );

	m_fUnk_13288               = 5.0f;
	pUnk->iRunning             = 0;
	pUnk->eState               = Enemy::ENEMYSTATE_ACTIVE;
	pUnk->iProjectilesInFlight = 0;
	pUnk->fStillTime           = 0;
	pUnk->bUnk4F               = a_bUnk;

	MakeEnemyRetreat( TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pUnk ) );

	if ( !pUnk->bUnk4F )
	{
		TINT iSlot = GetUnitStartBit( pUnk, m_aStages[ m_iStage ].iStageType );

		if ( iSlot > -1 && iSlot < 30 )
			CT_SetBit( m_auiCowSideReservedBits, iSlot );

		pUnk->bUnk4D = TTRUE;
	}

	PlayEnemySound( TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pUnk ), 361, 366, 373 );

	pUnit->GetAnimatableObject()->KillAllAnimations();
	pUnit->GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );

	if ( !CT_IsMode23( iMode ) )
		m_fMusicTimer = m_fMusicDelay;

	return TTRUE;
}
