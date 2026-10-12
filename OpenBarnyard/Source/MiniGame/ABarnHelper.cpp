#include "pch.h"
#include "ABarnHelper.h"
#include "ABarnExploreMinigameState.h"
#include "AGopherShopInterface.h"
#include "ARandom.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitType.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Units/AUnitFacialComponent.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Sound/AVOStream.h"
#include "Sound/ASoundManager.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "SoundBank/nb.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// off_007739b8
static const TCHAR* s_aAnimNames[ ABarnHelper::ANIM_NUMOF ] = {
	"pool_idle",
	"pool_look",
	"pinball_hammerpull",
	"pinball_play",
	"whack_look",
	"whack_idle",
	"darts_idle",
	"darts_stepup",
	"darts_throw",
	"darts_stepdown",
	"mechman_idle",
	"cheer",
	"bar_drink",
	"bar_idle",
	"play_music",
	"talk",
	"holdcue",
	"holddrink",
	"dance1",
	"dance2",
	"dance3",
	"idle1",
	"idle2",
	"idle3",
	"idle4",
	"cards_idle",
	"cards_look",
	"lookl",
	"lookr",
	"idle_lp",
};

// The goals of the games the goat walks between (off_00773a30) and the items that place them (DAT_0071f598)
static const TCHAR* s_aGoatGoals[ ABarnHelper::MAX_NUM_GOAT_SPOTS ] = {
	"goat_foosball",
	"goat_pinball",
	"goat_pinball",
	"goat_darts",
	"goat_darts",
	"goat_darts",
	"goat_mechman",
	"goat_cards",
};

static const TINT s_aGoatItems[ ABarnHelper::MAX_NUM_GOAT_SPOTS ] = { 973, 974, 969, 970, 971, 968, 976, 0 };

// DAT_00782e98
T2Vector<TVector4, 6> ABarnHelper::ms_vecGoatPositions;

// DAT_00782e30
T2Vector<TVector4, 6> ABarnHelper::ms_vecGoatDirections;

// The VO set of the unit: its type name without "nb_"
static void CopyVOSetName( TCHAR* a_szOutName, TSIZE a_uiSize, AUnit* a_pUnit )
{
	T2String8::Copy( a_szOutName, a_pUnit->GetType()->GetName().GetString() + 3, a_uiSize );
}

static TRandom& GetRandom()
{
	return ARandom::GetSingleton()->m_oRandom;
}

// $Barnyard: FUNCTION 004a5ad0
ABarnHelper::ABarnHelper( AUnit* a_pPlayer )
{
	m_iPinballCue = -1;
	m_vecPinball.Clear();
	m_vecDarts.Clear();
	m_vecCards.Clear();
	m_iNumBandMembers     = 0;
	m_bPinballCuePlaying  = TFALSE;
	m_bDaisyBessyChatting = TFALSE;
	m_pPlayer             = a_pPlayer;
	m_iWildMikePosition   = 0;
	m_iGoatSpot           = 0;

	ms_vecGoatPositions.Clear();
	ms_vecGoatDirections.Clear();

	for ( TINT i = 0; i < ANIM_NUMOF; i++ )
		m_aAnims[ i ] = TPString8( s_aAnimNames[ i ] );

	m_oBessy.pUnit         = TNULL;
	m_oDaisy.pUnit         = TNULL;
	m_oPool.pUnit          = TNULL;
	m_oFreddy.pUnit        = TNULL;
	m_oCheer.pUnit         = TNULL;
	m_oOtis.pUnit          = TNULL;
	m_oGoat.pUnit          = TNULL;
	m_oWildMike.pObject    = TNULL;
	m_oWildMikeBox.pObject = TNULL;

	for ( TINT i = 0; i < BAND_NUMOF; i++ )
		m_aBand[ i ].pUnit = TNULL;
}

// $Barnyard: FUNCTION 0049d3d0
ABarnHelper::~ABarnHelper()
{
}

// T2Vector<Activity, 2>::Clear
// $Barnyard: FUNCTION 004a49d0
static void ClearActivities( T2Vector<ABarnHelper::Activity, 2>& a_rVector )
{
	for ( TINT i = 0; i < a_rVector.Size(); i++ )
		TUtil::MemClear( &a_rVector[ i ], sizeof( ABarnHelper::Activity ) );

	a_rVector.Clear();
}

// $Barnyard: FUNCTION 004a4cd0
void ABarnHelper::Reset()
{
	m_oGoat.pUnit          = TNULL;
	m_oCheer.pUnit         = TNULL;
	m_oFreddy.pUnit        = TNULL;
	m_oPool.pUnit          = TNULL;
	m_oWildMikeBox.pObject = TNULL;
	m_oWildMike.pObject    = TNULL;

	ClearActivities( m_vecPinball );
	ClearActivities( m_vecDarts );

	for ( TINT i = 0; i < BAND_NUMOF; i++ )
		m_aBand[ i ].pUnit = TNULL;
}

// $Barnyard: FUNCTION 004a6350
void ABarnHelper::Update( TFLOAT a_fDeltaTime )
{
	if ( ABarnExploreMinigameState::ms_iFloor != ABarnExploreMinigameState::FLOOR_LOFT )
	{
		if ( m_oOtis.pUnit )
			UpdateOtis( a_fDeltaTime, IsPlayerNear( m_oOtis.pUnit ) );

		if ( m_oDaisy.pUnit )
		{
			TBOOL bNear = TVector4::DistanceSq( *m_pPlayer->GetSteer()->GetPosition(), m_vecDaisyBessyPosition ) < 6.0f;
			UpdateDaisyBessy( a_fDeltaTime, m_oDaisy, bNear );
			UpdateDaisyBessy( a_fDeltaTime, m_oBessy, bNear );
		}

		// The band plays once it's (almost) complete
		if ( m_iNumBandMembers == 5 || m_iNumBandMembers == 4 )
		{
			for ( TINT i = 0; i < BAND_NUMOF; i++ )
			{
				if ( !m_aBand[ i ].pUnit )
					continue;

				AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, m_aBand[ i ].pUnit )->GetAnimatableObject();

				if ( m_aBand[ i ].iState == 0 && pObject->IsAnimationFinished( m_aAnims[ ANIM_PLAY_MUSIC ] ) )
					pObject->PlayAnimation( m_aAnims[ ANIM_PLAY_MUSIC ] );
			}
		}

		if ( m_oWildMike.pObject )
			UpdateWildMike( a_fDeltaTime );
	}
	else
	{
		if ( m_oPool.pUnit )
			UpdatePool( a_fDeltaTime, IsPlayerNear( m_oPool.pUnit ) );

		for ( TINT i = 0; i < m_vecCards.Size(); i++ )
			UpdateCards( a_fDeltaTime, m_vecCards[ i ], IsPlayerNear( m_vecCards[ i ].pUnit ) );

		for ( TINT i = 0; i < m_vecPinball.Size(); i++ )
			UpdatePinball( a_fDeltaTime, m_vecPinball[ i ] );

		if ( m_oFreddy.pUnit )
			UpdateFreddy( a_fDeltaTime );

#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: the original updates both players as soon as there's one, with a single player the second one is garbage
		if ( m_vecDarts.Size() == 2 )
#else
		if ( m_vecDarts.Size() > 0 )
#endif
		{
			UpdateDarts( a_fDeltaTime, m_vecDarts[ 0 ], m_vecDarts[ 1 ] );
			UpdateDarts( a_fDeltaTime, m_vecDarts[ 1 ], m_vecDarts[ 0 ] );
		}

		if ( m_oCheer.pUnit )
			UpdateCheer( a_fDeltaTime );

		if ( m_oGoat.pUnit )
			UpdateGoat( a_fDeltaTime, IsPlayerNear( m_oGoat.pUnit ) );

		if ( m_oWildMikeBox.pObject )
			UpdateWildMikeBox( a_fDeltaTime );
	}
}

TBOOL ABarnHelper::IsPlayerNear( AUnit* a_pUnit ) const
{
	return TVector4::DistanceSq( *m_pPlayer->GetSteer()->GetPosition(), *a_pUnit->GetSteer()->GetPosition() ) < 6.0f;
}

// $Barnyard: FUNCTION 004a4a80
void ABarnHelper::AddPool( AUnit* a_pUnit )
{
	m_oPool.pUnit  = a_pUnit;
	m_oPool.iState = 0;
	CopyVOSetName( m_oPool.szName, sizeof( m_oPool.szName ), a_pUnit );
}

// $Barnyard: FUNCTION 004a4be0
void ABarnHelper::AddPinball( AUnit* a_pUnit )
{
	Activity& rActivity = *m_vecPinball.PushBack();
	rActivity.iState    = 0;
	rActivity.pUnit     = a_pUnit;

	// The sound of the pinball machines
	if ( !m_bPinballCuePlaying )
	{
		m_iPinballCue        = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::NB_PINBALL, *a_pUnit->GetSteer()->GetPosition() );
		m_bPinballCuePlaying = TTRUE;
	}
}

// $Barnyard: FUNCTION 004a4800
void ABarnHelper::AddFreddy( AUnit* a_pUnit )
{
	m_oFreddy.iState = 0;
	m_oFreddy.pUnit  = a_pUnit;
}

// $Barnyard: FUNCTION 004a4a30
void ABarnHelper::AddOtis( AUnit* a_pUnit )
{
	m_oOtis.pUnit  = a_pUnit;
	m_oOtis.iState = 0;
	m_oOtis.bFlag  = TFALSE;
	CopyVOSetName( m_oOtis.szName, sizeof( m_oOtis.szName ), a_pUnit );
}

// $Barnyard: FUNCTION 004a4c40
void ABarnHelper::AddDarts( AUnit* a_pUnit )
{
	Activity& rActivity = *m_vecDarts.PushBack();

	// The second player waits for the first one to throw
	rActivity.iState = ( m_vecDarts.Size() != 1 ) ? 2 : 0;
	rActivity.pUnit  = a_pUnit;
}

// $Barnyard: FUNCTION 004a4940
void ABarnHelper::AddCheer( AUnit* a_pUnit )
{
	m_oCheer.iState = 0;
	m_oCheer.pUnit  = a_pUnit;
	m_oCheer.fTimer = 0.0f;
}

// $Barnyard: FUNCTION 004a5030
void ABarnHelper::AddBandMember( AUnit* a_pUnit )
{
	const TCHAR* szType = a_pUnit->GetType()->GetName().GetString();
	TINT         iIndex;

	if ( TStringManager::String8CompareNoCase( szType, "nb_duke" ) == 0 )
		iIndex = BAND_DUKE;
	else if ( TStringManager::String8CompareNoCase( szType, "nb_everet" ) == 0 )
		iIndex = BAND_EVERET;
	else if ( TStringManager::String8CompareNoCase( szType, "nb_generic_cow" ) == 0 )
		iIndex = BAND_COW;
	else if ( TStringManager::String8CompareNoCase( szType, "nb_generic_goat" ) == 0 )
		iIndex = BAND_GOAT;
	else if ( TStringManager::String8CompareNoCase( szType, "nb_generic_horse" ) == 0 )
		iIndex = BAND_HORSE;
	else
	{
		// NOTE: the original resets the member at an uninitialised index here
		return;
	}

	m_aBand[ iIndex ].pUnit = a_pUnit;
	m_iNumBandMembers++;

	m_aBand[ iIndex ].iState = 0;
	m_aBand[ iIndex ].fTimer = 0.0f;
}

// $Barnyard: FUNCTION 004a5400
void ABarnHelper::AddCards( AUnit* a_pUnit )
{
	Activity& rActivity = *m_vecCards.PushBack();
	rActivity.pUnit     = a_pUnit;
	rActivity.fTimer    = 0.0f;
	rActivity.iState    = 0;
}

// $Barnyard: FUNCTION 004a4ad0
void ABarnHelper::AddDaisyBessy( AUnit* a_pUnit )
{
	TString8  strDaisy( "nb_daisy" );
	Activity& rActivity = ( strDaisy == a_pUnit->GetType()->GetName().GetString() ) ? m_oDaisy : m_oBessy;

	rActivity.iState = 0;
	rActivity.pUnit  = a_pUnit;
	rActivity.fTimer = 0.0f;
	rActivity.bFlag  = TFALSE;
	CopyVOSetName( rActivity.szName, sizeof( rActivity.szName ), a_pUnit );

	// The player chats with them when close to the spot between them
	if ( m_oDaisy.pUnit && m_oBessy.pUnit )
		m_vecDaisyBessyPosition.Lerp4( *m_oDaisy.pUnit->GetSteer()->GetPosition(), *m_oBessy.pUnit->GetSteer()->GetPosition(), 0.5f );
}

// $Barnyard: FUNCTION 004a4c70
TBOOL ABarnHelper::HasGoatSpots()
{
	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();

	for ( TINT i = 0; i < MAX_NUM_GOAT_SPOTS; i++ )
	{
		TVector4 vecPosition;
		pNetwork->FindGoalOrFirst( s_aGoatGoals[ i ] )->GetPosition( vecPosition );

		if ( AGopherShopInterface::HasBought( s_aGoatItems[ i ] ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004a5c90
void ABarnHelper::AddGoat( AUnit* a_pUnit )
{
	// The spots of the bought games
	for ( TINT i = 0; i < MAX_NUM_GOAT_SPOTS; i++ )
	{
		AInteriorGoal* pGoal = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->FindGoalOrFirst( s_aGoatGoals[ i ] );

		TVector4 vecPosition;
		pGoal->GetPosition( vecPosition );

		if ( !AGopherShopInterface::HasBought( s_aGoatItems[ i ] ) )
			continue;

		if ( ms_vecGoatPositions.Find( vecPosition ) == ms_vecGoatPositions.End() )
		{
			ms_vecGoatDirections.PushBack( pGoal->m_vForward );
			ms_vecGoatPositions.PushBack( vecPosition );
		}
	}

	m_oGoat.pUnit  = a_pUnit;
	m_oGoat.iState = 0;
	m_oGoat.fTimer = 0.0f;
	m_iGoatSpot    = -1;

	TSTATICCAST( AAnimatedUnit, m_oGoat.pUnit )->GetAnimatableObject()->PlayAnimation( TPS8D( "idle_lp" ) );
}

// $Barnyard: FUNCTION 004a5e20
void ABarnHelper::AddWildMike( AAnimatableObject* a_pObject )
{
	m_oWildMike.pObject = a_pObject;
	m_oWildMike.iState  = 0;
	m_oWildMike.fTimer  = 0.0f;

	a_pObject->GetModelInstance()->GetPosition( m_vecWildMikePosition );
	m_iWildMikePosition = 0;
}

// $Barnyard: FUNCTION 004a4960
void ABarnHelper::AddWildMikeBox( AAnimatableObject* a_pObject )
{
	m_oWildMikeBox.pObject = a_pObject;
	m_oWildMikeBox.iState  = 0;
	m_oWildMikeBox.fTimer  = 0.0f;
}

// $Barnyard: FUNCTION 004a4980
void ABarnHelper::SetBandVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < BAND_NUMOF; i++ )
	{
		if ( m_aBand[ i ].pUnit )
			TSTATICCAST( AVisibleUnit, m_aBand[ i ].pUnit )->SetVisible( a_bVisible );
	}
}

// $Barnyard: FUNCTION 004a49b0
void ABarnHelper::SetWildMikeUpdating( TBOOL a_bUpdating )
{
	if ( m_oWildMike.pObject )
		m_oWildMike.pObject->SetSkeletonUpdating( a_bUpdating, TTRUE );
}

// $Barnyard: FUNCTION 004a4de0
void ABarnHelper::UpdatePool( TFLOAT a_fDeltaTime, TBOOL a_bNear )
{
	TRandom&           rRandom   = GetRandom();
	AAnimatableObject* pObject   = TSTATICCAST( AAnimatedUnit, m_oPool.pUnit )->GetAnimatableObject();
	AVOStream*         pVOStream = AVOStream::GetSingleton();

	switch ( m_oPool.iState )
	{
		case 0:
			if ( pObject->IsAnimationFinished( m_aAnims[ ANIM_POOL_LOOK ] ) || m_oPool.bFlag )
			{
				m_oPool.bFlag = TFALSE;
				pObject->PlayAnimation( m_aAnims[ ANIM_POOL_IDLE ] );
				m_oPool.iState = 1;
				m_oPool.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
			}
			break;
		case 1:
			m_oPool.fTimer -= a_fDeltaTime;

			if ( m_oPool.fTimer < 0.0f )
			{
				if ( rRandom.GetFloatMinMax( 0.0f, 2.0f ) <= 1.0f )
				{
					// Talk, with a VO line if the player is close
					if ( a_bNear && !pVOStream->IsPlayRequested() && !pVOStream->IsPlaying() )
					{
						pVOStream->PlaySet( m_oPool.szName, "chat", *m_oPool.pUnit->GetSteer()->GetPosition() );
						m_oPool.bFlag = TTRUE;
					}

					pObject->PlayAnimation( m_aAnims[ ANIM_TALK ] );
					pObject->PlayAnimation( m_aAnims[ ANIM_HOLDCUE ] );
					m_oPool.iState = 2;
				}
				else
				{
					m_oPool.iState = 0;
					pObject->PlayAnimation( m_aAnims[ ANIM_POOL_LOOK ] );
				}
			}
			break;
		case 2:
			if ( !pVOStream->IsPlayRequested() )
			{
				if ( m_oPool.bFlag )
				{
					if ( pVOStream->IsPlaying() )
						return;
				}
				else if ( !pObject->IsAnimationFinished( m_aAnims[ ANIM_TALK ] ) )
				{
					return;
				}

				pObject->FinishAnimation( m_aAnims[ ANIM_TALK ] );
				pObject->FinishAnimation( m_aAnims[ ANIM_HOLDCUE ] );
				m_oPool.iState = 1;
			}
			break;
	}
}

// $Barnyard: FUNCTION 004a4d30
void ABarnHelper::UpdatePinball( TFLOAT a_fDeltaTime, Activity& a_rActivity )
{
	TRandom&           rRandom = GetRandom();
	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, a_rActivity.pUnit )->GetAnimatableObject();

	switch ( a_rActivity.iState )
	{
		case 0:
			pObject->PlayAnimation( m_aAnims[ ANIM_PINBALL_HAMMERPULL ] );
			a_rActivity.iState = 1;
			break;
		case 1:
			if ( pObject->IsAnimationFinished( m_aAnims[ ANIM_PINBALL_HAMMERPULL ] ) )
			{
				a_rActivity.iState = 2;
				a_rActivity.fTimer = rRandom.GetFloatMinMax( 5.0f, 10.0f );
				pObject->PlayAnimation( m_aAnims[ ANIM_PINBALL_PLAY ] );
			}
			break;
		case 2:
			a_rActivity.fTimer -= a_fDeltaTime;

			if ( a_rActivity.fTimer < 0.0f )
				a_rActivity.iState = 0;
			break;
	}
}

// $Barnyard: FUNCTION 004a4f90
void ABarnHelper::UpdateFreddy( TFLOAT a_fDeltaTime )
{
	TRandom&           rRandom = GetRandom();
	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, m_oFreddy.pUnit )->GetAnimatableObject();

	if ( m_oFreddy.iState == 0 )
	{
		if ( pObject->IsAnimationFinished( m_aAnims[ ANIM_WHACK_LOOK ] ) )
		{
			pObject->PlayAnimation( m_aAnims[ ANIM_WHACK_IDLE ] );
			m_oFreddy.iState = 1;
			m_oFreddy.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
		}
	}
	else if ( m_oFreddy.iState == 1 )
	{
		m_oFreddy.fTimer -= a_fDeltaTime;

		if ( m_oFreddy.fTimer < 0.0f )
		{
			m_oFreddy.iState = 0;
			pObject->PlayAnimation( m_aAnims[ ANIM_WHACK_LOOK ] );
		}
	}
}

// $Barnyard: FUNCTION 004a4820
void ABarnHelper::UpdateDarts( TFLOAT a_fDeltaTime, Activity& a_rActivity, const Activity& a_rcOther )
{
	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, a_rActivity.pUnit )->GetAnimatableObject();

	switch ( a_rActivity.iState )
	{
		case 0:
			pObject->PlayAnimation( m_aAnims[ ANIM_DARTS_IDLE ] );
			a_rActivity.iState = 1;
			break;
		case 1:
			// Steps up once the other one steps down
			if ( a_rcOther.iState == 4 )
			{
				pObject->PlayAnimation( m_aAnims[ ANIM_DARTS_STEPUP ] );
				a_rActivity.iState = 2;
			}
			break;
		case 2:
			if ( pObject->IsAnimationFinished( m_aAnims[ ANIM_DARTS_STEPUP ] ) )
			{
				// Three darts
				a_rActivity.iState = 3;
				pObject->PlayAnimation( m_aAnims[ ANIM_DARTS_THROW ] );
				a_rActivity.fTimer = 2.0f;
			}
			break;
		case 3:
			if ( pObject->IsAnimationFinished( m_aAnims[ ANIM_DARTS_THROW ] ) )
			{
				a_rActivity.fTimer -= 1.0f;

				if ( a_rActivity.fTimer >= 0.0f )
				{
					pObject->PlayAnimation( m_aAnims[ ANIM_DARTS_THROW ] );
				}
				else
				{
					a_rActivity.iState = 4;
					pObject->PlayAnimation( m_aAnims[ ANIM_DARTS_STEPDOWN ] );
				}
			}
			break;
		case 4:
			if ( pObject->IsAnimationFinished( m_aAnims[ ANIM_DARTS_STEPDOWN ] ) )
				a_rActivity.iState = 0;
			break;
	}
}

// $Barnyard: FUNCTION 004a5180
void ABarnHelper::UpdateCheer( TFLOAT a_fDeltaTime )
{
	TRandom&           rRandom = GetRandom();
	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, m_oCheer.pUnit )->GetAnimatableObject();

	if ( m_oCheer.iState == 0 )
	{
		m_oCheer.fTimer -= a_fDeltaTime;

		if ( m_oCheer.fTimer >= 0.0f )
			return;

		pObject->PlayAnimation( m_aAnims[ ANIM_MECHMAN_IDLE ] );
		m_oCheer.iState = 1;
		m_oCheer.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
	}
	else if ( m_oCheer.iState == 1 )
	{
		m_oCheer.fTimer -= a_fDeltaTime;

		if ( m_oCheer.fTimer < 0.0f )
		{
			pObject->PlayAnimation( m_aAnims[ ANIM_CHEER ] );
			m_oCheer.iState = 0;
			m_oCheer.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
		}
	}
}

// $Barnyard: FUNCTION 004a59a0
void ABarnHelper::UpdateCards( TFLOAT a_fDeltaTime, Activity& a_rActivity, TBOOL a_bNear )
{
	TRandom&              rRandom = GetRandom();
	AAnimatableObject*    pObject = TSTATICCAST( AAnimatedUnit, a_rActivity.pUnit )->GetAnimatableObject();
	AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, a_rActivity.pUnit->FindComponent( &TGetClass( AUnitFacialComponent ) ) );

	switch ( a_rActivity.iState )
	{
		case 0:
			a_rActivity.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
			a_rActivity.iState = 1;
			pObject->PlayAnimation( m_aAnims[ ANIM_CARDS_IDLE ] );
			break;
		case 1:
			a_rActivity.fTimer -= a_fDeltaTime;

			if ( a_rActivity.fTimer >= 0.0f )
			{
				// Looks at the player
				if ( a_bNear && !pFacial->IsLooking() && rRandom.GetFloatMinMax( 0.0f, 1.0f ) > 0.3f )
				{
					pFacial->LookAtUnitHead( m_pPlayer, -1.0f );
					a_rActivity.fTimer = 5.0f;
				}
			}
			else
			{
				pFacial->StopLook();
				pObject->FinishAnimation( m_aAnims[ ANIM_CARDS_IDLE ] );
				pObject->PlayAnimation( m_aAnims[ ANIM_CARDS_LOOK ] );
				a_rActivity.iState = 2;
			}
			break;
		case 2:
			if ( pObject->IsAnimationFinished( m_aAnims[ ANIM_CARDS_LOOK ] ) )
				a_rActivity.iState = 0;
			break;
	}
}

// $Barnyard: FUNCTION 004a54c0
void ABarnHelper::UpdateGoat( TFLOAT a_fDeltaTime, TBOOL a_bNear )
{
	TRandom&              rRandom = GetRandom();
	AAnimatableObject*    pObject = TSTATICCAST( AAnimatedUnit, m_oGoat.pUnit )->GetAnimatableObject();
	AInteriorSteer*       pSteer  = TSTATICCAST( AInteriorSteer, m_oGoat.pUnit->GetSteer() );
	AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, m_oGoat.pUnit->FindComponent( &TGetClass( AUnitFacialComponent ) ) );

	switch ( m_oGoat.iState )
	{
		case 0:
			m_oGoat.fTimer -= a_fDeltaTime;

			if ( m_oGoat.fTimer >= 0.0f )
			{
				if ( a_bNear && !pFacial->IsLooking() && rRandom.GetFloatMinMax( 0.0f, 1.0f ) > 0.3f )
				{
					pFacial->LookAtUnitHead( m_pPlayer, -1.0f );
					m_oGoat.fTimer = 5.0f;
				}
			}
			else
			{
				// Walks to another game
				pFacial->StopLook();

				if ( ms_vecGoatPositions.Size() <= 1 )
				{
					m_iGoatSpot = 0;
					pSteer->SeekPosition( ms_vecGoatPositions[ 0 ] );
					pSteer->Command_FaceDirection( ms_vecGoatDirections[ 0 ] );
				}
				else
				{
					TINT iSpot;

					do
						iSpot = rRandom.GetInt( ms_vecGoatPositions.Size() );
					while ( m_iGoatSpot == iSpot );

					m_iGoatSpot = iSpot;
					pSteer->SeekPosition( ms_vecGoatPositions[ iSpot ] );
				}

				m_oGoat.iState = 2;
				m_oGoat.fTimer = 5.0f;
			}
			break;
		case 1:
			m_oGoat.fTimer -= a_fDeltaTime;

			if ( m_oGoat.fTimer < 0.0f )
			{
				pObject->FinishAnimation( m_aAnims[ ANIM_CHEER ] );
				m_oGoat.fTimer = rRandom.GetFloatMinMax( 1.0f, 3.0f );
				m_oGoat.iState = 0;
			}
			break;
		case 2:
			if ( pSteer->IsCommandFinished() )
			{
				// Arrived, faces the game
				pFacial->StopLook();
				pSteer->Command_FaceDirection( ms_vecGoatDirections[ m_iGoatSpot ] );
				m_oGoat.iState = 3;
			}
			else if ( !pFacial->IsLooking() && m_oGoat.fTimer > 0.0f )
			{
				if ( rRandom.GetFloatMinMax( 0.0f, 1.0f ) > 0.3f )
				{
					pFacial->LookAtUnitHead( m_pPlayer, -1.0f );
					m_oGoat.fTimer = 5.0f;
				}
			}
			else if ( pFacial->IsLooking() )
			{
				m_oGoat.fTimer -= a_fDeltaTime;

				if ( m_oGoat.fTimer < 0.0f )
					pFacial->StopLook();
			}
			break;
		case 3:
			if ( pSteer->IsCommandFinished() )
			{
				pObject->PlayAnimation( m_aAnims[ ANIM_CHEER ] );
				m_oGoat.fTimer = rRandom.GetFloatMinMax( 1.0f, 3.0f );
				m_oGoat.iState = 1;
			}
			break;
	}
}

// $Barnyard: FUNCTION 004a6250
void ABarnHelper::UpdateWildMikeBox( TFLOAT a_fDeltaTime )
{
	TRandom&           rRandom = GetRandom();
	AAnimatableObject* pObject = m_oWildMikeBox.pObject;

	if ( m_oWildMikeBox.iState == 0 )
	{
		// One of the idles, or nothing
		TINT iAnim           = rRandom.GetInt( 7 );
		m_oWildMikeBox.bFlag = TFALSE;

		if ( iAnim < 4 )
		{
			m_strWildMikeBoxAnim = m_aAnims[ ANIM_IDLE1 + iAnim ];
			pObject->PlayAnimation( m_strWildMikeBoxAnim );
			m_oWildMikeBox.bFlag = TTRUE;
		}

		m_oWildMikeBox.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
		m_oWildMikeBox.iState = 1;
	}
	else if ( m_oWildMikeBox.iState == 1 )
	{
		m_oWildMikeBox.fTimer -= a_fDeltaTime;

		if ( m_oWildMikeBox.fTimer < 0.0f )
		{
			if ( m_oWildMikeBox.bFlag )
				pObject->FinishAnimation( m_strWildMikeBoxAnim );

			m_oWildMikeBox.iState = 0;
		}
	}
}

// $Barnyard: FUNCTION 004a5220
void ABarnHelper::UpdateOtis( TFLOAT a_fDeltaTime, TBOOL a_bNear )
{
	TRandom&           rRandom   = GetRandom();
	AAnimatableObject* pObject   = TSTATICCAST( AAnimatedUnit, m_oOtis.pUnit )->GetAnimatableObject();
	AVOStream*         pVOStream = AVOStream::GetSingleton();

	switch ( m_oOtis.iState )
	{
		case 0:
			if ( !pObject->IsAnimationFinished( m_aAnims[ ANIM_BAR_DRINK ] ) )
				return;

			pObject->PlayAnimation( m_aAnims[ ANIM_BAR_IDLE ] );
			m_oOtis.iState = 1;
			m_oOtis.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
			break;
		case 1:
			m_oOtis.fTimer -= a_fDeltaTime;

			if ( m_oOtis.fTimer >= 0.0f )
				return;

			if ( rRandom.GetFloatMinMax( 0.0f, 2.0f ) <= 1.0f )
			{
				if ( a_bNear && !pVOStream->IsPlayRequested() && !pVOStream->IsPlaying() )
				{
					pVOStream->PlaySet( m_oOtis.szName, "chat", *m_oOtis.pUnit->GetSteer()->GetPosition() );
					m_oOtis.bFlag = TTRUE;
				}

				pObject->PlayAnimation( m_aAnims[ ANIM_TALK ] );
				pObject->PlayAnimation( m_aAnims[ ANIM_HOLDDRINK ] );
				m_oOtis.iState = 2;
			}
			else
			{
				pObject->PlayAnimation( m_aAnims[ ANIM_BAR_DRINK ] );
				m_oOtis.iState = 0;
				m_oOtis.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
			}
			break;
		case 2:
			if ( !pVOStream->IsPlayRequested() )
			{
				if ( m_oOtis.bFlag )
				{
					if ( pVOStream->IsPlaying() )
						return;
				}
				else if ( !pObject->IsAnimationFinished( m_aAnims[ ANIM_TALK ] ) )
				{
					return;
				}

				pObject->FinishAnimation( m_aAnims[ ANIM_TALK ] );
				pObject->FinishAnimation( m_aAnims[ ANIM_HOLDDRINK ] );
				m_oOtis.bFlag  = TFALSE;
				m_oOtis.iState = 0;
			}
			break;
	}
}

// $Barnyard: FUNCTION 004a5790
void ABarnHelper::UpdateDaisyBessy( TFLOAT a_fDeltaTime, Activity& a_rActivity, TBOOL a_bNear )
{
	TRandom&              rRandom   = GetRandom();
	AAnimatableObject*    pObject   = TSTATICCAST( AAnimatedUnit, a_rActivity.pUnit )->GetAnimatableObject();
	AUnitFacialComponent* pFacial   = TSTATICCAST( AUnitFacialComponent, a_rActivity.pUnit->FindComponent( &TGetClass( AUnitFacialComponent ) ) );
	AVOStream*            pVOStream = AVOStream::GetSingleton();

	if ( a_rActivity.iState != 0 )
	{
		if ( a_rActivity.iState != 1 || pVOStream->IsPlayRequested() )
			return;

		a_rActivity.fTimer -= a_fDeltaTime;

		if ( a_rActivity.bFlag )
		{
			if ( pVOStream->IsPlaying() )
				return;
		}
		else if ( a_rActivity.fTimer >= 0.0f )
		{
			return;
		}

		// Done chatting
		a_rActivity.bFlag     = TFALSE;
		m_bDaisyBessyChatting = TFALSE;
		a_rActivity.fTimer    = rRandom.GetFloatMinMax( 2.0f, 5.0f );

		if ( pObject->HasAnimation( m_aAnims[ ANIM_TALK ] ) )
			pObject->FinishAnimation( m_aAnims[ ANIM_TALK ] );

		pObject->PlayAnimation( m_aAnims[ ANIM_IDLE_LP ] );
		a_rActivity.iState = 0;
		return;
	}

	a_rActivity.fTimer -= a_fDeltaTime;

	if ( a_rActivity.fTimer >= 0.0f )
	{
		if ( a_bNear && !pFacial->IsLooking() && rRandom.GetFloatMinMax( 0.0f, 1.0f ) > 0.3f )
		{
			pFacial->LookAtUnitHead( m_pPlayer, -1.0f );
			a_rActivity.fTimer = 5.0f;
		}

		return;
	}

	pFacial->StopLook();

	if ( m_bDaisyBessyChatting )
	{
		// The other one talks
		a_rActivity.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
		return;
	}

	if ( pObject->HasAnimation( m_aAnims[ ANIM_TALK ] ) )
		pObject->PlayAnimation( m_aAnims[ ANIM_TALK ] );

	a_rActivity.bFlag  = TFALSE;
	a_rActivity.fTimer = 3.0f;

	if ( a_bNear && !pVOStream->IsPlayRequested() && !pVOStream->IsPlaying() )
	{
		pVOStream->PlaySet( a_rActivity.szName, "chat", *a_rActivity.pUnit->GetSteer()->GetPosition() );
		a_rActivity.bFlag = TTRUE;
	}

	a_rActivity.iState    = 1;
	m_bDaisyBessyChatting = TTRUE;
}

// $Barnyard: FUNCTION 004a5eb0
void ABarnHelper::UpdateWildMike( TFLOAT a_fDeltaTime )
{
	TRandom&           rRandom = GetRandom();
	AAnimatableObject* pObject = m_oWildMike.pObject;

	switch ( m_oWildMike.iState )
	{
		case 0:
		{
			// Dances
			TINT iAnim = rRandom.GetInt( 3 );

			// dance2 only in the middle of the stage
			if ( iAnim == 1 && ( m_vecWildMikePosition.x < 11.0f || m_vecWildMikePosition.x > 17.0f ) )
				iAnim = ( rRandom.GetInt( 2 ) != 1 ) ? 2 : 0;

			m_strWildMikeAnim     = m_aAnims[ ANIM_DANCE1 + iAnim ];
			ms_bWildMikeKillAnims = ( iAnim == 0 );

			pObject->PlayAnimation( m_strWildMikeAnim );
			m_oWildMike.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
			m_oWildMike.iState = 1;
			m_oWildMike.bFlag  = ( iAnim == 2 );
			break;
		}
		case 1:
			m_oWildMike.fTimer -= a_fDeltaTime;

			if ( m_oWildMike.fTimer >= 0.0f )
			{
				if ( m_oWildMike.bFlag && pObject->IsAnimationFinished( m_strWildMikeAnim ) )
					pObject->PlayAnimation( m_strWildMikeAnim );
			}
			else
			{
				// Walks to another position of the stage
				m_oWildMike.fTimer = 0.0f;
				pObject->FinishAnimation( m_strWildMikeAnim );

				if ( ms_bWildMikeKillAnims )
					pObject->KillAllAnimations();

				TINT iPosition;

				if ( m_iWildMikePosition != 0 )
					iPosition = 0;
				else
					iPosition = ( rRandom.GetFloatMinMax( 0.0f, 1.0f ) <= 0.5f ) ? 2 : 1;

				m_vecWildMikeTarget = m_vecWildMikePosition;
				m_vecWildMikeTarget.Lerp4( m_vecWildMikeTarget, m_aWildMikePositions[ iPosition ], rRandom.GetFloatMinMax( 0.2f, 1.0f ) );

				m_iWildMikePosition = iPosition;
				m_oWildMike.iState  = 2;
				m_oWildMike.fTimer  = 0.0f;
				pObject->PlayAnimation( m_aAnims[ ANIM_DANCE1 ] );
			}
			break;
		case 2:
			m_oWildMike.fTimer += a_fDeltaTime;

			if ( m_oWildMike.fTimer <= 2.0f )
			{
				TVector4 vecPosition;
				vecPosition.Lerp4( m_vecWildMikePosition, m_vecWildMikeTarget, m_oWildMike.fTimer * 0.5f );
				pObject->GetModelInstance()->SetTranslation( vecPosition.AsVector3() );
			}
			else
			{
				m_vecWildMikePosition = m_vecWildMikeTarget;
				pObject->GetModelInstance()->SetTranslation( m_vecWildMikePosition.AsVector3() );
				pObject->FinishAnimation( m_aAnims[ ANIM_DANCE1 ] );
				m_oWildMike.iState = 0;
			}
			break;
	}
}
