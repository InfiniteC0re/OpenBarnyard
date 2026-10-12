#include "pch.h"
#include "ABarnYardPoolMiniGameState.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Cameras/APoolCamera.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/AGameStateController.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnitPoolBall.h"
#include "Units/AUnitPoolBfa.h"
#include "Units/AUnitPoolCue.h"
#include "UnitMovement/Steering/APoolBallSteer.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Toshi/T2String8.h>
#include "SoundBank/pool.h"

#ifdef BARNYARD_COMMUNITY_PATCH
#  include <windows.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static APoolBallSteer* GetBallSteer( AUnit* a_pUnit )
{
	return TSTATICCAST( APoolBallSteer, a_pUnit->GetSteer() );
}

// $Barnyard: FUNCTION 004d5520
void ABarnYardPoolMiniGameState::ResetGame()
{
	m_iOtherTeam        = 1;
	m_iNextTeam         = 1;
	m_bSubmitHighScore  = TTRUE;
	m_eState            = STATE_START;
	m_iCurrentTeam      = 0;
	m_iWinner           = 6;
	m_iPlayerWon        = 6;
	m_iUnkD7C           = 6;
	m_eFirstGroup       = BALLTYPE_NONE;
	m_eFirstHitBallType = BALLTYPE_NONE;
	m_aTeams[ 0 ].Reset();
	m_aTeams[ 1 ].Reset();
	m_iShotSound       = -1;
	m_iBallSound       = -1;
	m_iCushionSound    = -1;
	m_iPocketSound     = -1;
	m_bPottedGroupB    = TFALSE;
	m_bPottedGroupA    = TFALSE;
	m_bPottedCueBall   = TFALSE;
	m_bPottedBlackBall = TFALSE;
	m_bOpenTable       = TTRUE;
	m_bAimForwardOnly  = TTRUE;
	m_eBallsState      = BALLSSTATE_MOVING;
	m_iUnkE40          = 0;

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	TVector4    vTable;
	TVector4    vBlack;
	TVector4    vWhite;
	TQuaternion quatRotation;
	pTerrain->GetLocatorPosAndRot( "bn_pool_table_add", vTable, quatRotation );
	pTerrain->GetLocatorPosAndRot( "ball_black", vBlack, quatRotation );
	pTerrain->GetLocatorPosAndRot( "ball_white", vWhite, quatRotation );
	pTerrain->GetLocatorPosAndRot( "hole_01", m_avPockets[ 0 ], quatRotation );
	pTerrain->GetLocatorPosAndRot( "hole_02", m_avPockets[ 1 ], quatRotation );
	pTerrain->GetLocatorPosAndRot( "hole_03", m_avPockets[ 2 ], quatRotation );
	pTerrain->GetLocatorPosAndRot( "hole_04", m_avPockets[ 3 ], quatRotation );
	pTerrain->GetLocatorPosAndRot( "hole_05", m_avPockets[ 4 ], quatRotation );
	pTerrain->GetLocatorPosAndRot( "hole_06", m_avPockets[ 5 ], quatRotation );

	// The bounds of the table
	m_fTableMinX = vWhite.x;
	m_fTableMaxX = vWhite.x;
	m_fTableMinZ = vWhite.z;
	m_fTableMaxZ = vWhite.z;

	for ( TINT i = NUM_POCKETS - 1; i >= 0; i-- )
	{
		if ( m_avPockets[ i ].x > m_fTableMaxX )
			m_fTableMaxX = m_avPockets[ i ].x;

		if ( m_avPockets[ i ].x < m_fTableMinX )
			m_fTableMinX = m_avPockets[ i ].x;

		if ( m_avPockets[ i ].z > m_fTableMaxZ )
			m_fTableMaxZ = m_avPockets[ i ].z;

		if ( m_avPockets[ i ].z < m_fTableMinZ )
			m_fTableMinZ = m_avPockets[ i ].z;
	}

	TFLOAT fMargin  = m_fPocketRadius + m_fPocketRadius;
	m_vCueBallStart = vWhite;
	m_fTableMinX -= fMargin;
	m_fTableMaxX += fMargin;
	m_fTableMinZ -= fMargin;
	m_fTableMaxZ += fMargin;

	m_vForward.Set( vBlack.x - vWhite.x, vBlack.y - vWhite.y, vBlack.z - vWhite.z, vBlack.w );
	m_vForward.Normalise();
	m_fBallInHandRadius = 0.5f;

	// Everything is at the height of the cloth
	TFLOAT fHeight = pTerrain->GetGroundHeight( vBlack, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) - 0.06f;
	vBlack.y       = fHeight;
	vWhite.y       = fHeight;
	vTable.y       = fHeight;

	for ( TINT i = NUM_POCKETS - 1; i >= 0; i-- )
		m_avPockets[ i ].y = fHeight;

	TVector4 vOffset;
	vOffset.Multiply( m_vForward, 0.03f );
	RackBalls( vBlack, TVector4( vWhite.x - vOffset.x, vWhite.y - vOffset.y, vWhite.z - vOffset.z, vWhite.w ), 0.05f );

	m_pCueBall   = TSTATICCAST( AUnitPoolBall, GetBallUnit( CUE_BALL ) );
	m_pBlackBall = TSTATICCAST( AUnitPoolBall, GetBallUnit( BLACK_BALL ) );

	for ( TINT i = NUM_BALLS - 1; i >= 0; i-- )
	{
		AUnitPoolBall*  pBall  = TSTATICCAST( AUnitPoolBall, GetBallUnit( i ) );
		APoolBallSteer* pSteer = GetBallSteer( pBall );

		TVector4 vPosition = *pSteer->GetPosition();
		pBall->SetStartPosition( vPosition );
		pBall->SetPotted( TFALSE );
		pSteer->ResetHitBall();
		pSteer->ResetHitCushion();
		pSteer->ResetHitPocket();

		// Potted balls go under the table
		vPosition.y += 3.0f;
		pSteer->SetPottedPosition( vPosition );

		TVector4 vCenter( vBlack.x + vWhite.x, vBlack.y + vWhite.y, vBlack.z + vWhite.z, vBlack.w );
		vCenter.Divide( 2.0f );
		pSteer->SetTableCenter( vCenter );
		pSteer->SetPockets( NUM_POCKETS, m_avPockets, m_fPocketRadius );
		pSteer->SetState( APoolBallSteer::STATE_STOPPED );
	}

	for ( TINT i = NUM_BALLS - 1; i >= 0; i-- )
		m_aShadows[ i ]->GetUnit()->SetSteerPosition( *GetBallUnit( i )->GetSteer()->GetPosition() );

	for ( TINT i = NUM_BALLS - 1; i >= 0; i-- )
		TSTATICCAST( AAnimatedUnit, GetBallUnit( i ) )->GetModelInstance()->SetEnvironmentMap( "FX\\env_shiny.tga", 0, TNULL );

	m_vAimTarget = vBlack;
	m_pAimTarget = TSTATICCAST( AUnitPoolBfa, m_pBfa->GetUnit() );

	m_aCue[ 0 ]->GetUnit()->SetSteerPosition( TVector4( 14.0f, 5.69f, 35.0f, 1.0f ) );
	m_pCue = TSTATICCAST( AUnitPoolCue, m_aCue[ 0 ]->GetUnit() );
	m_pCue->SetCueBallPosition( m_pCueBall->GetPosition() );
	m_pCue->SetTarget( m_vAimTarget );
	m_pCue->SetMaxPower( 15.0f );
	m_pCue->SetVisible( TFALSE );
	m_pCue->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

	// The camera views: 0-3 around the table...
	m_pCamera->ClearViews();
	m_aViews[ 0 ].vPosition.Set( vWhite.x, vWhite.y - 3.0f, vWhite.z - 2.0f, 1.0f );
	m_aViews[ 0 ].vLookAt.Set( vWhite.x, vWhite.y, vWhite.z + 0.75f, 1.0f );
	m_aViews[ 1 ].vPosition.Set( vTable.x + 1.0f, vTable.y - 4.25f, vTable.z, 1.0f );
	m_aViews[ 1 ].vLookAt.Set( vTable.x, vTable.y, vTable.z, 1.0f );
	m_aViews[ 2 ].vPosition.Set( vBlack.x, vBlack.y - 4.0f, vBlack.z + 2.5f, 1.0f );
	m_aViews[ 2 ].vLookAt.Set( vBlack.x, vBlack.y, vBlack.z - 0.25f, 1.0f );
	m_aViews[ 3 ].vPosition.Set( vTable.x - 1.0f, vTable.y - 4.25f, vTable.z, 1.0f );
	m_aViews[ 3 ].vLookAt.Set( vTable.x, vTable.y, vTable.z, 1.0f );
	m_pCamera->AddView( &m_aViews[ 0 ].vPosition, &m_aViews[ 0 ].vLookAt, 0 );
	m_pCamera->AddView( &m_aViews[ 1 ].vPosition, &m_aViews[ 1 ].vLookAt, 0 );
	m_pCamera->AddView( &m_aViews[ 2 ].vPosition, &m_aViews[ 2 ].vLookAt, 0 );
	m_pCamera->AddView( &m_aViews[ 3 ].vPosition, &m_aViews[ 3 ].vLookAt, 0 );

	// ...and 4-6 behind the cue (see UpdateCueViews)
	m_aViews[ 4 ] = m_aViews[ 0 ];
	m_aViews[ 5 ] = m_aViews[ 0 ];
	m_aViews[ 6 ] = m_aViews[ 0 ];
	m_pCamera->AddView( &m_aViews[ 4 ].vPosition, &m_aViews[ 4 ].vLookAt, 1 );
	m_pCamera->AddView( &m_aViews[ 5 ].vPosition, &m_aViews[ 5 ].vLookAt, 1 );
	m_pCamera->AddView( &m_aViews[ 6 ].vPosition, &m_aViews[ 6 ].vLookAt, 1 );
	m_pCamera->SetView( 5 );
	m_iSavedView = m_pCamera->GetCurrentView();

	m_oPowerBar.SetValue( 0.0f, TFALSE );

	if ( m_iCurrentTeam == 0 )
		ALocale::FormatTags( m_wszText, TEXT_SIZE - 2, LOCALE_PLAYER_TURN, PPropertyValue( const_cast<TWCHAR*>( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString( 0 ) ), TFALSE ) );
	else
		ALocale::FormatTags( m_wszText, TEXT_SIZE - 2, LOCALE_OPPONENT_TURN, PPropertyValue( const_cast<TWCHAR*>( m_wszOpponentName ), TFALSE ) );

	m_oHud.SetMessage( m_wszText );

	// The computer
	m_oAI.SetBalls( NUM_BALLS, m_aBalls, 0.045f );
	m_oAI.SetPockets( NUM_POCKETS, m_avPockets, m_fPocketRadius );
	m_oAI.SetMaxShotPower( 15.0f );
	m_oAI.SetDifficulty( m_iDifficulty );
	m_oAI.SetCueAndBlackBall( m_pCueBall, m_pBlackBall );
	m_oAI.SetForward( m_vForward );

	m_vShotVelocity = TVector4::VEC_ZERO;
	m_vAIAimPoint   = TVector4::VEC_ZERO;
	m_fAIShotPower  = 0.0f;

	// The teams
	switch ( m_iNumPlayers )
	{
		case 1:
			// The player against Duke
			m_aTeams[ 0 ].SetNumPlayers( 1 );
			m_aTeams[ 1 ].SetNumPlayers( 1 );
			m_aTeams[ 0 ].SetPlayer( 0, 0 );
			m_aTeams[ 1 ].SetPlayer( 0, PLAYER_AI );
			break;
		case 2:
			if ( m_iNumAIPlayers == 2 )
			{
				m_aTeams[ 0 ].SetNumPlayers( 2 );
				m_aTeams[ 1 ].SetNumPlayers( 2 );
				m_aTeams[ 0 ].SetPlayer( 0, 0 );
				m_aTeams[ 0 ].SetPlayer( 1, PLAYER_AI );
				m_aTeams[ 1 ].SetPlayer( 0, 1 );
				m_aTeams[ 1 ].SetPlayer( 1, PLAYER_AI );
			}
			else if ( m_iNumAIPlayers == 3 )
			{
				m_aTeams[ 0 ].SetNumPlayers( 2 );
				m_aTeams[ 1 ].SetNumPlayers( 2 );
				m_aTeams[ 0 ].SetPlayer( 0, 0 );
				m_aTeams[ 0 ].SetPlayer( 1, 1 );
				m_aTeams[ 1 ].SetPlayer( 0, PLAYER_AI );
				m_aTeams[ 1 ].SetPlayer( 1, PLAYER_AI );
			}
			else
			{
				m_aTeams[ 0 ].SetNumPlayers( 1 );
				m_aTeams[ 1 ].SetNumPlayers( 1 );
				m_aTeams[ 0 ].SetPlayer( 0, 0 );
				m_aTeams[ 1 ].SetPlayer( 0, 1 );
			}
			break;
		case 3:
			m_aTeams[ 0 ].SetNumPlayers( 2 );
			m_aTeams[ 1 ].SetNumPlayers( 2 );
			m_aTeams[ 0 ].SetPlayer( 0, 0 );
			m_aTeams[ 0 ].SetPlayer( 1, 1 );
			m_aTeams[ 1 ].SetPlayer( 0, 2 );
			m_aTeams[ 1 ].SetPlayer( 1, PLAYER_AI );
			break;
		case 4:
			m_aTeams[ 0 ].SetNumPlayers( 2 );
			m_aTeams[ 1 ].SetNumPlayers( 2 );
			m_aTeams[ 0 ].SetPlayer( 0, 0 );
			m_aTeams[ 0 ].SetPlayer( 1, 1 );
			m_aTeams[ 1 ].SetPlayer( 0, 2 );
			m_aTeams[ 1 ].SetPlayer( 1, 3 );
			break;
	}

	m_fGameOverTimer = 0.0f;
	m_oHud.SetNumLitBalls( 0, 0 );
	m_oHud.SetNumLitBalls( 1, 0 );
	m_oHud.SetTeamsVisible( TTRUE );
	m_oHud.Reset();
	CreateProps();

	m_fMessageTimer    = 0.0f;
	m_bGroupsAnnounced = TFALSE;
	m_eNextState       = STATE_NONE;
}

// $Barnyard: FUNCTION 004d4510
void ABarnYardPoolMiniGameState::RackBalls( TVector4 a_vBlack, TVector4 a_vCueBall, TFLOAT a_fBallRadius )
{
	TFLOAT fRowDistance = TMath::Sqrt( 3.0f ) * a_fBallRadius;
	TFLOAT fRadius2     = a_fBallRadius + a_fBallRadius;

	GetBallUnit( 0 )->SetSteerPosition( a_vCueBall );

	// The apex of the triangle
	GetBallUnit( 1 )->SetSteerPosition( TVector4( a_vBlack.x, a_vBlack.y, a_vBlack.z - ( fRowDistance + fRowDistance ), a_vBlack.w ) );

	// Second row
	GetBallUnit( 3 )->SetSteerPosition( TVector4( a_vBlack.x - a_fBallRadius, a_vBlack.y, a_vBlack.z - fRowDistance, a_vBlack.w ) );
	GetBallUnit( 9 )->SetSteerPosition( TVector4( a_vBlack.x + a_fBallRadius, a_vBlack.y, a_vBlack.z - fRowDistance, a_vBlack.w ) );

	// Third row, the black ball in the middle
	GetBallUnit( 10 )->SetSteerPosition( TVector4( a_vBlack.x - fRadius2, a_vBlack.y, a_vBlack.z, a_vBlack.w ) );
	GetBallUnit( 8 )->SetSteerPosition( TVector4( a_vBlack.x, a_vBlack.y, a_vBlack.z, a_vBlack.w ) );
	GetBallUnit( 7 )->SetSteerPosition( TVector4( fRadius2 + a_vBlack.x, a_vBlack.y, a_vBlack.z, a_vBlack.w ) );

	// Fourth row
	TFLOAT fRadius3 = a_fBallRadius * 3.0f;
	TFLOAT fRow4    = fRowDistance + a_vBlack.z;
	GetBallUnit( 5 )->SetSteerPosition( TVector4( a_vBlack.x - fRadius3, a_vBlack.y, fRow4, a_vBlack.w ) );
	GetBallUnit( 15 )->SetSteerPosition( TVector4( a_vBlack.x - a_fBallRadius, a_vBlack.y, fRow4, a_vBlack.w ) );
	GetBallUnit( 2 )->SetSteerPosition( TVector4( a_vBlack.x + a_fBallRadius, a_vBlack.y, fRow4, a_vBlack.w ) );
	GetBallUnit( 12 )->SetSteerPosition( TVector4( fRadius3 + a_vBlack.x, a_vBlack.y, fRow4, a_vBlack.w ) );

	// Fifth row
	TFLOAT fRadius4 = a_fBallRadius * 4.0f;
	TFLOAT fRow5    = ( fRowDistance + fRowDistance ) + a_vBlack.z;
	GetBallUnit( 14 )->SetSteerPosition( TVector4( a_vBlack.x - fRadius4, a_vBlack.y, fRow5, a_vBlack.w ) );
	GetBallUnit( 4 )->SetSteerPosition( TVector4( a_vBlack.x - fRadius2, a_vBlack.y, fRow5, a_vBlack.w ) );
	GetBallUnit( 13 )->SetSteerPosition( TVector4( a_vBlack.x, a_vBlack.y, fRow5, a_vBlack.w ) );
	GetBallUnit( 11 )->SetSteerPosition( TVector4( fRadius2 + a_vBlack.x, a_vBlack.y, fRow5, a_vBlack.w ) );
	GetBallUnit( 6 )->SetSteerPosition( TVector4( fRadius4 + a_vBlack.x, a_vBlack.y, fRow5, a_vBlack.w ) );
}

// $Barnyard: FUNCTION 004d1e90
void ABarnYardPoolMiniGameState::NextTurn()
{
	TINT iTeam = m_iCurrentTeam;
	m_aTeams[ iTeam ].NextPlayer();
	m_iCurrentTeam = m_iOtherTeam;

	if ( m_aTeams[ m_iCurrentTeam ].GetNumPotted() == 7 )
	{
		m_iNextTeam  = iTeam;
		m_iOtherTeam = iTeam;
	}
	else
	{
		m_iNextTeam  = m_iOtherTeam;
		m_iOtherTeam = iTeam;
		m_oHud.ShowTurn( m_iNextTeam );
	}

	m_oHud.SetTitleText( ALocaleManager::Interface()->GetString( LOCALE_TURN_OVER ) );
}

// $Barnyard: FUNCTION 004d2960
void ABarnYardPoolMiniGameState::OnTeamBallPotted()
{
	if ( m_iCurrentTeam == 0 )
		m_oHud.SetTitleText( ALocaleManager::Interface()->GetString( LOCALE_POTTED ) );

	if ( !m_bGroupsAnnounced && ( m_aTeams[ 0 ].GetNumPotted() > 0 || m_aTeams[ 1 ].GetNumPotted() > 0 ) )
	{
		m_bGroupsAnnounced = TTRUE;

		if ( m_iCurrentTeam == 0 )
		{
			if ( m_bPottedGroupB == TTRUE )
				m_oHud.SetInfoText( ALocaleManager::Interface()->GetString( LOCALE_GROUP_B ) );
			else if ( m_bPottedGroupA == TTRUE )
				m_oHud.SetInfoText( ALocaleManager::Interface()->GetString( LOCALE_GROUP_A ) );
		}
	}

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->PlayCueAtPosition( soundbank::POOL_CLAP, m_pBlackBall->GetPosition() );

	// NOTE: retail plays 295 a second time when the computer potted the ball (299 for the players)
	if ( GetCurrentTeam().GetCurrentPlayer() == PLAYER_AI )
		pSoundManager->PlayCueAtPosition( soundbank::POOL_CLAP, m_pBlackBall->GetPosition() );
	else
		pSoundManager->PlayCueAtPosition( soundbank::POOL_DUKE_BALL, m_pBlackBall->GetPosition() );
}

// $Barnyard: FUNCTION 004d4900
void ABarnYardPoolMiniGameState::EndShot()
{
	m_eState = STATE_NONE;

	if ( m_bPottedBlackBall == TTRUE )
	{
		// The game is over
		if ( m_eFirstHitBallType != BALLTYPE_BLACK || GetCurrentTeam().GetNumPotted() != 7 || m_bPottedGroupB == TTRUE || m_bPottedGroupA == TTRUE || m_bPottedCueBall == TTRUE )
		{
			// Foul on the black ball, the other team wins
			m_oHud.SetInfoText( ALocaleManager::Interface()->GetString( LOCALE_FOUL_BLACK_BALL ) );
			m_oHud.SetTitleText( ALocaleManager::Interface()->GetString( LOCALE_TURN_OVER ) );
			m_iWinner    = m_iOtherTeam;
			m_eState     = STATE_MESSAGE;
			m_eNextState = STATE_GAME_DECIDED;
		}
		else
		{
			m_iWinner = m_iCurrentTeam;
			m_aTeams[ m_iCurrentTeam ].OnBallPotted( BALLTYPE_BLACK );
			m_oHud.SetNumLitBalls( m_iWinner, m_aTeams[ m_iWinner ].GetNumPotted() );
			m_eState = STATE_GAME_DECIDED;
		}

		m_iPlayerWon = m_iWinner == 0;
		return;
	}

	if ( m_bPottedCueBall == TTRUE )
	{
		// Ball in hand for the other team
		m_oHud.SetInfoText( ALocaleManager::Interface()->GetString( LOCALE_FOUL_CUE_BALL ) );
		NextTurn();
		m_pCueBall->SetPosition( m_pCueBall->GetStartPosition() );
		m_eState        = STATE_MESSAGE;
		m_eNextState    = STATE_BALL_IN_HAND;
		m_fMessageTimer = 0.0f;
		return;
	}

	TINT iFoulText;

	if ( m_eFirstHitBallType == BALLTYPE_NONE )
	{
		// Didn't hit any ball
		iFoulText = LOCALE_FOUL_NO_HIT;
		goto foul_text;
	}

	if ( m_bPottedGroupB == TTRUE )
	{
		if ( m_bPottedGroupA == TTRUE )
			goto foul;

		if ( GetCurrentTeam().GetBallType() != BALLTYPE_GROUP_B )
		{
			// Potted a ball of the other team
			m_eState        = STATE_MESSAGE;
			m_eNextState    = STATE_START_TURN;
			m_fMessageTimer = 0.0f;
			m_oHud.SetInfoText( ALocaleManager::Interface()->GetString( LOCALE_GROUP_B ) );
			NextTurn();
			goto end;
		}

		if ( GetCurrentTeam().GetBallType() != m_eFirstHitBallType && m_bOpenTable != TTRUE )
			goto foul_hit;

		goto potted;
	}

	if ( m_bPottedGroupA == TTRUE )
	{
		if ( GetCurrentTeam().GetBallType() != BALLTYPE_GROUP_A )
		{
			// Potted a ball of the other team
			iFoulText = LOCALE_GROUP_A;
			goto foul_text;
		}

		if ( GetCurrentTeam().GetBallType() != m_eFirstHitBallType && m_bOpenTable != TTRUE )
			goto foul_hit;

		goto potted;
	}

	// Nothing potted
	if ( GetCurrentTeam().GetBallType() == m_eFirstHitBallType || GetCurrentTeam().GetBallType() == BALLTYPE_NONE )
	{
		// Legal shot, the next team plays
		if ( m_iCurrentTeam != m_iNextTeam )
			m_aTeams[ m_iCurrentTeam ].NextPlayer();

		m_iCurrentTeam = m_iNextTeam;
		m_iOtherTeam   = m_iNextTeam == 0;
		m_iNextTeam    = m_iNextTeam == 0;

		if ( m_iCurrentTeam == 1 )
			ASoundManager::GetSingleton()->PlayCue( soundbank::POOL_DUKE_TAUNT );

		goto end;
	}

	if ( m_eFirstHitBallType != BALLTYPE_BLACK || GetCurrentTeam().GetNumPotted() != 7 )
		goto foul_hit;

	{
		// Hit the black ball (the team is on it) without potting it
		m_aTeams[ m_iCurrentTeam ].NextPlayer();
		m_iNextTeam    = m_iCurrentTeam;
		TINT iOther    = m_iOtherTeam;
		m_iCurrentTeam = iOther;
		m_iOtherTeam   = iOther == 0;
		goto end;
	}

foul_hit:
	// Hit a ball of the wrong type first
	switch ( m_eFirstHitBallType )
	{
		case BALLTYPE_BLACK:
			iFoulText = LOCALE_FOUL_HIT_BLACK;
			break;
		case BALLTYPE_GROUP_A:
			iFoulText = LOCALE_FOUL_HIT_GROUP_A;
			break;
		case BALLTYPE_GROUP_B:
			iFoulText = LOCALE_FOUL_HIT_GROUP_B;
			break;
		default:
			// NOTE: retail would show locale string -1 here, but the first hit ball can't be the cue ball
			iFoulText = -1;
			break;
	}

foul_text:
	m_oHud.SetInfoText( ALocaleManager::Interface()->GetString( iFoulText ) );

foul:
	// The other team plays
	NextTurn();
	m_eState        = STATE_MESSAGE;
	m_eNextState    = STATE_START_TURN;
	m_fMessageTimer = 0.0f;
	goto end;

potted:
	// The team keeps playing
	OnTeamBallPotted();

	if ( m_iCurrentTeam == 0 )
	{
		m_eState        = STATE_MESSAGE_POTTED;
		m_eNextState    = STATE_START_TURN;
		m_fMessageTimer = 0.0f;
	}

	m_bOpenTable = TFALSE;

end:
	if ( m_iCurrentTeam == m_iNextTeam && GetCurrentTeam().GetNumPotted() == 7 )
		m_iNextTeam = m_iOtherTeam;

	if ( m_eState == STATE_NONE )
		m_eState = STATE_START_TURN;

	m_pAimTarget->SetVisible( TFALSE );
	m_pCue->SetVisible( TFALSE );
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" runs place the cue ball and let the planner of the computer aim and shoot for the player
void ABarnYardPoolMiniGameState::UpdateDevAutoPlay( TFLOAT a_fDeltaTime )
{
	static TBOOL s_bChecked = TFALSE;
	static TBOOL s_bEnabled = TFALSE;

	if ( !s_bChecked )
	{
		s_bChecked = TTRUE;
		s_bEnabled = T2String8::FindString( GetCommandLineA(), "-devautoplay" ) != TNULL;
	}

	if ( !s_bEnabled )
		return;

	if ( m_eState != STATE_HUMAN_AIM && m_eState != STATE_HUMAN_PLACE_BALL )
	{
		m_fDevAutoPlayTimer = 0.0f;
		return;
	}

	m_fDevAutoPlayTimer += a_fDeltaTime;

	if ( m_fDevAutoPlayTimer < 1.0f )
		return;

	m_fDevAutoPlayTimer = 0.0f;

	if ( m_eState == STATE_HUMAN_PLACE_BALL )
	{
		if ( m_pCueBall->IsPlacementValid() )
		{
			TINFO( "[devautoplay] placed the cue ball\n" );
			m_eState      = STATE_START_TURN;
			m_fInputDelay = 0.1f;
		}

		return;
	}

	m_oAI.SetTargetBallType( GetCurrentTeam().GetTargetBallType() );
	m_vShotVelocity = m_oAI.GetShot( TTRUE );
	m_vAimTarget    = m_oAI.GetAimPoint();
	m_eState        = STATE_HUMAN_SHOT;
	m_iDevAutoPlayShots++;

	TINFO( "[devautoplay] shot %d by team %d (target %d), velocity (%.2f, %.2f), potted %d - %d\n", m_iDevAutoPlayShots, m_iCurrentTeam, GetCurrentTeam().GetTargetBallType(), m_vShotVelocity.x, m_vShotVelocity.z, m_aTeams[ 0 ].GetNumPotted(), m_aTeams[ 1 ].GetNumPotted() );
}

#endif // BARNYARD_COMMUNITY_PATCH

// $Barnyard: FUNCTION 004d6a60
void ABarnYardPoolMiniGameState::UpdateGame( TFLOAT a_fDeltaTime )
{
	UpdateProps( a_fDeltaTime );
	m_fInputDelay -= a_fDeltaTime;
	// NOTE: retail calls DOES_NOTHING (005acd80) on m_oAimLine here

	if ( m_eState == STATE_RESULTS )
	{
		// Starts a new game once the results are gone
		if ( !FindChildState( &TGetClass( AAnticGameOverState ) ) )
			ResetGame();

		return;
	}

	UpdateAimGuide( a_fDeltaTime );

	if ( m_eState == STATE_START )
		m_eState = STATE_BALL_IN_HAND;

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	// Sounds of the balls and the group of the first potted ball
	m_eBallsState = BALLSSTATE_STOPPED;

	for ( TINT i = NUM_BALLS - 1; i >= 0; i-- )
	{
		AUnitPoolBall*  pBall  = TSTATICCAST( AUnitPoolBall, GetBallUnit( i ) );
		APoolBallSteer* pSteer = GetBallSteer( pBall );

		if ( pSteer->HasHitBall() == TTRUE )
		{
			pSteer->ResetHitBall();
			m_iBallSound = pSoundManager->PlayCueAtPosition( soundbank::POOL_BALL_BALL, *pSteer->GetPosition() );
		}

		if ( pSteer->HasHitCushion() == TTRUE )
		{
			pSteer->ResetHitCushion();
			m_iCushionSound = pSoundManager->PlayCueAtPosition( soundbank::POOL_BALL_CUSHION, *pSteer->GetPosition() );
		}

		if ( pSteer->HasHitPocket() == TTRUE )
		{
			pSteer->ResetHitPocket();
			m_iPocketSound = pSoundManager->PlayCueAtPosition( soundbank::POOL_BALL_CUSHION, *pSteer->GetPosition() );
		}

		switch ( pSteer->GetState() )
		{
			case APoolBallSteer::STATE_MOVING:
			case APoolBallSteer::STATE_FALLING_POCKET:
				m_eBallsState = BALLSSTATE_MOVING;
				break;
			case APoolBallSteer::STATE_POTTED:
				if ( pBall->GetBallType() != BALLTYPE_CUE && pBall->GetBallType() != BALLTYPE_BLACK && m_eFirstGroup == BALLTYPE_NONE )
				{
					// The first potted ball decides the groups
					m_eFirstGroup = pBall->GetBallType();
					m_aTeams[ m_iCurrentTeam ].SetBallType( m_eFirstGroup );
					m_aTeams[ m_iOtherTeam ].SetOppositeBallType( m_eFirstGroup );
					m_oHud.SetBallColour( m_iCurrentTeam, m_aTeams[ m_iCurrentTeam ].GetBallType() );
					m_oHud.SetBallColour( m_iOtherTeam, m_aTeams[ m_iOtherTeam ].GetBallType() );
				}
				break;
			default:
				break;
		}
	}

	m_bPottedGroupB    = TFALSE;
	m_bPottedGroupA    = TFALSE;
	m_bPottedCueBall   = TFALSE;
	m_bPottedBlackBall = TFALSE;

	if ( m_eBallsState == BALLSSTATE_MOVING )
		m_iUnkE40 = 0;

	if ( m_eState == STATE_BALLS_ROLLING && m_eBallsState != BALLSSTATE_MOVING )
	{
		// Every ball has stopped, see what the shot did
		m_eFirstHitBallType = m_pCueBall->GetFirstHitBallType();

		if ( m_pCueBall->GetSteerState() == APoolBallSteer::STATE_POTTED )
			m_bPottedCueBall = TTRUE;

		if ( m_pBlackBall->GetSteerState() == APoolBallSteer::STATE_POTTED )
			m_bPottedBlackBall = TTRUE;

		for ( TINT i = NUM_BALLS - 1; i >= 0; i-- )
		{
			AUnitPoolBall* pBall = TSTATICCAST( AUnitPoolBall, GetBallUnit( i ) );

			if ( pBall->GetSteerState() == APoolBallSteer::STATE_POTTED && pBall->GetBallType() != BALLTYPE_CUE && pBall->GetBallType() != BALLTYPE_BLACK && pBall->IsPotted() == TFALSE )
			{
				pBall->SetPotted( TTRUE );
				m_aTeams[ 0 ].OnBallPotted( pBall->GetBallType() );
				m_aTeams[ 1 ].OnBallPotted( pBall->GetBallType() );
				m_oHud.SetNumLitBalls( 0, m_aTeams[ 0 ].GetNumPotted() );
				m_oHud.SetNumLitBalls( 1, m_aTeams[ 1 ].GetNumPotted() );

				if ( pBall->GetBallType() == BALLTYPE_GROUP_B )
					m_bPottedGroupB = TTRUE;
				else
					m_bPottedGroupA = TTRUE;
			}
		}
	}

	if ( m_eState == STATE_BALLS_ROLLING && m_eBallsState != BALLSSTATE_MOVING )
		EndShot();

#ifdef BARNYARD_COMMUNITY_PATCH
	UpdateDevAutoPlay( a_fDeltaTime );
#endif

	switch ( m_eState )
	{
		case STATE_GAME_DECIDED:
			if ( m_iWinner != 0 )
				pSoundManager->PlayCueAtPosition( soundbank::POOL_DUKE_WIN, m_pBlackBall->GetPosition() );
			else
				pSoundManager->PlayCueAtPosition( soundbank::POOL_PLAYER_WIN, m_pBlackBall->GetPosition() );

			m_pCue->SetVisible( TFALSE );
			m_oPowerBar.SetBarVisible( TFALSE );

			if ( m_iWinner != 0 )
				ALocale::FormatTags( m_wszText, TEXT_SIZE - 2, LOCALE_OPPONENT_WINS, PPropertyValue( const_cast<TWCHAR*>( m_wszOpponentName ), TFALSE ) );
			else
				ALocale::FormatTags( m_wszText, TEXT_SIZE - 2, LOCALE_PLAYER_WINS, PPropertyValue( const_cast<TWCHAR*>( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString( 0 ) ), TFALSE ) );

			m_oHud.SetTitleText( m_wszText );
			m_eState = STATE_GAME_OVER_WAIT;
			break;
		case STATE_GAME_OVER_WAIT:
			m_fGameOverTimer += a_fDeltaTime;

			if ( m_fGameOverTimer > 4.0f )
			{
				m_fGameOverTimer = 0.0f;
				m_eState         = STATE_SHOW_RESULTS;
			}
			break;
		case STATE_SHOW_RESULTS:
			m_oHud.SetTeamsVisible( TFALSE );
			m_eState = STATE_RESULTS;
			AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );
			return;
		case STATE_BALLS_ROLLING:
			m_pAimTarget->SetVisible( TFALSE );
			m_pCue->SetVisible( TFALSE );
			m_oPowerBar.SetBarVisible( TFALSE );
			break;
		case STATE_START_TURN:
			m_oHud.SetFirstTeamActive( m_iCurrentTeam == 0 );

			if ( m_pCamera->GetCurrentView() != 5 )
				m_pCamera->SetCurrentView( 5 );

			m_fGuideScale = -0.1f;
			m_pAimTarget->SetVisible( TFALSE );
			m_eState = STATE_HUMAN_START;

			if ( GetCurrentTeam().GetCurrentPlayer() == PLAYER_AI )
			{
				m_pCue->SetVisible( TFALSE );
				m_pAimTarget->SetVisible( TFALSE );
				m_eState  = STATE_AI_PLAN;
				m_bUnkD9A = TTRUE;
			}
			else
			{
				ALocale::FormatInputTags( m_wszText, TEXT_SIZE - 2, ALocaleManager::Interface()->GetString( LOCALE_HOW_TO_SHOOT ) );
				m_oHud.SetMessage( m_wszText );
			}
			break;
		case STATE_HUMAN_START:
			m_pCue->SetVisible( TTRUE );
			m_oPowerBar.SetBarVisible( TTRUE );
			m_oPowerBar.SetValue( m_pCue->GetPower() / m_pCue->GetMaxPower(), TFALSE );
			m_pCamera->SetCurrentView( m_iSavedView );
			m_bUnkD9A = TFALSE;
			m_eState  = STATE_HUMAN_AIM;
			break;
		case STATE_HUMAN_POWER:
			m_pCue->UpdatePowerOscillation( a_fDeltaTime * 10.0f );
			m_oPowerBar.SetValue( m_pCue->GetPower() / m_pCue->GetMaxPower(), TFALSE );
			break;
		case STATE_HUMAN_SHOT:
		case STATE_AI_SHOT:
			m_eState = STATE_SHOOT;
			break;
		case STATE_AI_PLAN:
			m_pCue->SetVisible( TFALSE );
			m_pCamera->SetCurrentView( m_iSavedView );
			m_oAI.SetTargetBallType( GetCurrentTeam().GetTargetBallType() );
			m_vShotVelocity = m_oAI.GetShot( TFALSE );
			m_vAIAimPoint   = m_oAI.GetAimPoint();
			m_vAimTarget    = m_oAI.GetAimPoint();
			m_fAIShotPower  = m_vShotVelocity.AsVector3().Magnitude();
			m_eState        = STATE_AI_WAIT;
			break;
		case STATE_AI_WAIT:
			m_eState = STATE_AI_SHOW_CUE;
			break;
		case STATE_AI_SHOW_CUE:
			m_pCue->SetVisible( TTRUE );
			m_pCamera->SetCurrentView( m_iSavedView );
			m_eState = STATE_AI_WAIT_CAMERA;
			break;
		case STATE_AI_WAIT_CAMERA:
			if ( m_pCamera->IsMoving() == TFALSE )
			{
				m_oPowerBar.SetBarVisible( TTRUE );
				m_pCue->SetVisible( TTRUE );
				m_pCue->ResetPower();
				m_eState = STATE_AI_POWER;
			}
			break;
		case STATE_AI_POWER:
			if ( m_pCue->GetPower() >= m_fAIShotPower )
			{
				m_eState = STATE_AI_SHOT;
			}
			else
			{
				m_pCue->UpdatePowerOscillation( a_fDeltaTime * 10.0f );
				m_oPowerBar.SetValue( m_pCue->GetPower() / m_pCue->GetMaxPower(), TFALSE );
			}
			break;
		case STATE_SHOOT:
			m_iUnkE40    = 0;
			m_iSavedView = m_pCamera->GetCurrentView();
			ChooseShotView( m_vShotVelocity );
			m_eState = STATE_CUE_STRIKE;
			m_pAimTarget->SetVisible( TFALSE );
			break;
		case STATE_CUE_STRIKE:
			m_pCue->UpdatePullBack( a_fDeltaTime * 300.0f, a_fDeltaTime );

			if ( m_pCue->GetPower() <= -2.0f && m_pCue->m_fPullBackSpeed < 0.0f )
			{
				// The cue hits the cue ball
				m_eState = STATE_CUE_FOLLOW;
				GetCurrentTeam().iNumShots++;
				m_pCueBall->GetSteer()->SetVelocity( m_vShotVelocity );
				m_iShotSound = pSoundManager->PlayCueAtPosition( soundbank::POOL_BALL_CUE, m_pCueBall->GetPosition() );

				AInputHandler::RumbleParams oRumble;
				oRumble.afUnk[ 0 ] = m_pCue->GetPower() / m_pCue->GetMaxPower() * 0.8f;
				oRumble.afUnk[ 1 ] = 0.0f;
				oRumble.afUnk[ 2 ] = 0.0f;
				oRumble.afUnk[ 3 ] = 1.0f;
				oRumble.afUnk[ 4 ] = 0.0f;
				oRumble.afUnk[ 5 ] = 1.0f;
				oRumble.afUnk[ 6 ] = 1.0f;
				oRumble.iUnk       = 1;
				AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );

				m_pCue->Shoot();
			}
			break;
		case STATE_CUE_FOLLOW:
			m_oPowerBar.SetBarVisible( TFALSE );
			m_oPowerBar.SetValue( 0.0f, TTRUE );
			m_oHud.SetMessage( TNULL );

			if ( m_pCue->IsReady() )
			{
				if ( m_eBallsState == BALLSSTATE_STOPPED )
				{
					m_eState          = STATE_BALLS_ROLLING;
					m_bAimForwardOnly = TFALSE;
					m_pCue->ResetPower();
				}

				m_pCue->SetVisible( TFALSE );
			}
			break;
		case STATE_BALL_IN_HAND:
		{
			TVector4 vOffset;
			vOffset.Multiply( m_vForward, 0.5f );
			TVector4 vStart = m_pCueBall->GetStartPosition();
			m_vAimTarget.Set( vStart.x + vOffset.x, vStart.y + vOffset.y, vStart.z + vOffset.z, vStart.w );

			m_bAimForwardOnly = TTRUE;
			m_oPowerBar.SetBarVisible( TFALSE );
			m_pAimTarget->SetVisible( TFALSE );
			m_pCue->SetVisible( TFALSE );
			m_eState = STATE_HUMAN_PLACE_BALL;

			if ( GetCurrentTeam().GetCurrentPlayer() == PLAYER_AI )
			{
				m_eState = STATE_AI_PLACE_BALL;
			}
			else
			{
				ALocale::FormatTags( m_wszText, TEXT_SIZE - 2, LOCALE_PLAYER_TURN, PPropertyValue( const_cast<TWCHAR*>( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString( 0 ) ), TFALSE ) );
				m_oHud.SetMessage( m_wszText );
			}

			m_bUnkD9B    = TFALSE;
			m_iSavedView = 5;
			m_pCamera->SetCurrentView( 0 );
			break;
		}
		case STATE_HUMAN_PLACE_BALL:
			m_oHud.SetFirstTeamActive( m_iCurrentTeam == 0 );
			break;
		case STATE_AI_PLACE_BALL:
			m_oHud.SetFirstTeamActive( m_iCurrentTeam == 0 );
			m_oAI.SetTargetBallType( GetCurrentTeam().GetTargetBallType() );
			m_oAI.PlaceCueBall( m_vCueBallStart, m_pCueBall->GetRadius(), m_fBallInHandRadius, m_vForward );
			m_eState = STATE_START_TURN;
			break;
		case STATE_MESSAGE:
		case STATE_MESSAGE_POTTED:
			m_fMessageTimer += a_fDeltaTime;

			if ( m_fMessageTimer >= 2.0f )
			{
				m_eState        = m_eNextState;
				m_eNextState    = STATE_NONE;
				m_fMessageTimer = 0.0f;
			}
			break;
		default:
			break;
	}

	m_pAimTarget->SetSteerPosition( m_vAimTarget );
	m_pCue->SetCueBallPosition( m_pCueBall->GetPosition() );
	m_pCue->SetTarget( m_vAimTarget );
	UpdateCueViews();
	m_oHud.Update( a_fDeltaTime );

	// The shadows follow the balls
	for ( TINT i = NUM_BALLS - 1; i >= 0; i-- )
	{
		TVector4 vPosition = *GetBallUnit( i )->GetSteer()->GetPosition();
		vPosition.z -= 0.02f;
		vPosition.x -= 0.02f;
		m_aShadows[ i ]->GetUnit()->SetSteerPosition( vPosition );
	}

	UpdateAim( a_fDeltaTime );
}
