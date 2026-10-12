#include "pch.h"
#include "ABarnChatHelper.h"
#include "ABarnExploreMinigameState.h"
#include "ARandom.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitType.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Animation/AAnimatableObject.h"
#include "Sound/AVOStream.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// off_007739a8
static const TCHAR* s_aAnimNames[ ABarnChatHelper::ANIM_NUMOF ] = {
	"lookat",
	"talk",
	"idle_lp",
};

// $Barnyard: FUNCTION 004a4710
ABarnChatHelper::ABarnChatHelper( AUnit* a_pPlayer )
{
	m_bTalking = TFALSE;
	m_pPlayer  = a_pPlayer;

	for ( TINT i = 0; i < ANIM_NUMOF; i++ )
		m_aAnims[ i ] = TPString8( s_aAnimNames[ i ] );
}

// NOTE: a thunk to the clear of ACollisionModelRayHitList (004cace0), which has entries of the same size
// $Barnyard: FUNCTION 004a44d0
void ABarnChatHelper::Reset()
{
	for ( TINT i = 0; i < m_vecChatters.Size(); i++ )
		TUtil::MemClear( &m_vecChatters[ i ], sizeof( Chatter ) );

	m_vecChatters.Clear();
}

// $Barnyard: FUNCTION 004a43d0
void ABarnChatHelper::AddUnit( AUnit* a_pUnit )
{
	Chatter oChatter;
	oChatter.iState = 0;
	oChatter.pUnit  = a_pUnit;
	oChatter.fTimer = 0.0f;
	T2String8::Copy( oChatter.szName, a_pUnit->GetType()->GetName().GetString() + 3, sizeof( oChatter.szName ) );

	TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject()->PlayAnimation( m_aAnims[ ANIM_IDLE_LP ] );
	m_vecChatters.PushBack( oChatter );

	// The group is complete, the player chats with them close to Eddy
	if ( m_vecChatters.Size() == MAX_NUM_CHATTERS )
	{
		for ( TINT i = 0; i < m_vecChatters.Size(); i++ )
		{
			if ( TStringManager::String8Compare( m_vecChatters[ i ].szName, "eddy" ) == 0 )
				m_vecEddyPosition = *m_vecChatters[ i ].pUnit->GetSteer()->GetPosition();
		}
	}
}

// $Barnyard: FUNCTION 004a4650
void ABarnChatHelper::Update( TFLOAT a_fDeltaTime )
{
	TBOOL bNear = TVector4::DistanceSq( *m_pPlayer->GetSteer()->GetPosition(), m_vecEddyPosition ) < 6.0f;

	if ( ABarnExploreMinigameState::ms_iFloor != ABarnExploreMinigameState::FLOOR_LOFT )
	{
		for ( TINT i = 0; i < m_vecChatters.Size(); i++ )
			UpdateChatter( a_fDeltaTime, m_vecChatters[ i ], bNear );
	}
}

// $Barnyard: FUNCTION 004a44e0
void ABarnChatHelper::UpdateChatter( TFLOAT a_fDeltaTime, Chatter& a_rChatter, TBOOL a_bNear )
{
	TRandom&           rRandom   = ARandom::GetSingleton()->m_oRandom;
	AAnimatableObject* pObject   = TSTATICCAST( AAnimatedUnit, a_rChatter.pUnit )->GetAnimatableObject();
	AVOStream*         pVOStream = AVOStream::GetSingleton();

	if ( a_rChatter.iState != 0 )
	{
		if ( a_rChatter.iState == 1 && !pVOStream->IsPlayRequested() )
		{
			// Waits for the line (or the anim) to end
			if ( a_bNear )
			{
				if ( pVOStream->IsPlaying() )
					return;
			}
			else if ( !pObject->IsAnimationFinished( m_aAnims[ ANIM_TALK ] ) )
			{
				return;
			}

			m_bTalking        = TFALSE;
			a_rChatter.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
			pObject->FinishAnimation( m_aAnims[ ANIM_TALK ] );
			pObject->PlayAnimation( m_aAnims[ ANIM_IDLE_LP ] );
			a_rChatter.iState = 0;
		}

		return;
	}

	a_rChatter.fTimer -= a_fDeltaTime;

	if ( a_rChatter.fTimer >= 0.0f || m_pPlayer->HasInteraction() )
		return;

	if ( m_bTalking )
	{
		// Another one talks
		a_rChatter.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
		return;
	}

	pObject->PlayAnimation( m_aAnims[ ANIM_LOOKAT ] );
	pObject->PlayAnimation( m_aAnims[ ANIM_TALK ] );

	if ( a_bNear && !pVOStream->IsPlayRequested() && !pVOStream->IsPlaying() )
		pVOStream->PlaySet( a_rChatter.szName, "chat", *a_rChatter.pUnit->GetSteer()->GetPosition() );

	a_rChatter.iState = 1;
	m_bTalking        = TTRUE;
}
