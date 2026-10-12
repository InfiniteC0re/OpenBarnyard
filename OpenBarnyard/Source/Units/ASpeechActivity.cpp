#include "pch.h"
#include "ASpeechActivity.h"
#include "MiniGame/AGopherShopInterface.h"
#include "Helpers/ASimPersistData.h"
#include "Sound/ASoundManager.h"
#include "AUnit.h"
#include "AUnitPlayer.h"
#include "AInteractionComponent.h"
#include "ARandom.h"
#include "Locale/ALocaleManager.h"
#include "GameInterface/AGameStateController.h"
#include "AAnimatedUnit.h"
#include "AUnitType.h"
#include "UnitMovement/Steering/ASteer.h"
#include "AVisibleUnit.h"
#include "AInteractionSequence.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/ANamedAnimationSet.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ASpeechCameraHelper.h"
#include "Helpers/AConversationManager.h"
#include "Helpers/ASpeechDialogManager.h"
#include "Locale/ALocale.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Quests/AQuestManager.h"
#include "AUnitFacialComponent.h"
#include "Sound/AVOStream.h"
#include "GameInterface/ASimState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bed50, CreateObject 005bfd10
// NOTE: GetClass 005bed50, CreateObject 005bfd10, CreateObjectInPlace 005bfd70
TDEFINE_CLASS( ASpeechActivity );

TPSTRING8_DECLARE( talk_01 );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( bubblevisible );
TPSTRING8_DECLARE( bubblevisibleforplayer );
TPSTRING8_DECLARE( forcenoanim );
TPSTRING8_DECLARE( forcevisible );
TPSTRING8_DECLARE( sfx );
TPSTRING8_DECLARE( speechtype );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( idle_aux );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( none );

// $Barnyard: FUNCTION 005bf270
ASpeechActivity::ASpeechActivity()
    : m_iText( -1 )
    , m_uiFlags( FLAGS_BUBBLE_VISIBLE | FLAGS_BUBBLE_VISIBLE_FOR_PLAYER )
    , m_iNumTexts( 0 )
    , m_iCamera( 1 )
    , m_iSfx( -1 )
    , m_pPlayer( TNULL )
    , m_pPlayerFacial( TNULL )
{
	m_eCategory     = AInteractionSequence::CATEGORY_PRIMARY;
	m_strTalkAnim   = TPS8( talk_01 );
	m_strListenAnim = TPS8( idle_lp );

	TUtil::MemSet( m_aiTexts, 0xff, sizeof( m_aiTexts ) );
}

// $Barnyard: FUNCTION 005bf370
// $Barnyard: FUNCTION 005bfdb0
ASpeechActivity::~ASpeechActivity()
{
}

// $Barnyard: FUNCTION 005bf410
TBOOL ASpeechActivity::Create( const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const PBPropertyValue* pValue = it->GetValue();
		const TCHAR*           szName = it->GetName().GetString();

		switch ( szName[ 0 ] )
		{
			case 'b':
			{
				TPString8 strName( szName );

				if ( strName == TPS8( bubblevisibleforplayer ) )
				{
					if ( pValue->GetRaw() )
						m_uiFlags |= FLAGS_BUBBLE_VISIBLE_FOR_PLAYER;
					else
						m_uiFlags &= ~FLAGS_BUBBLE_VISIBLE_FOR_PLAYER;
				}
				else if ( strName == TPS8( bubblevisible ) )
				{
					if ( pValue->GetRaw() )
						m_uiFlags |= FLAGS_BUBBLE_VISIBLE;
					else
						m_uiFlags &= ~FLAGS_BUBBLE_VISIBLE;
				}

				break;
			}
			case 'c':
				if ( TStringManager::String8Compare( szName, "camera" ) == 0 )
					m_iCamera = TINT8( TREINTERPRETCAST( TUINTPTR, pValue->GetRaw() ) );

				break;
			case 'f':
			{
				TPString8 strName( szName );

				if ( strName == TPS8( forcenoanim ) )
				{
					// NOTE: retail never clears this one
					if ( pValue->GetRaw() )
						m_uiFlags |= FLAGS_FORCE_NO_ANIM;
				}
				else if ( strName == TPS8( forcevisible ) )
				{
					if ( pValue->GetRaw() )
						m_uiFlags |= FLAGS_FORCE_VISIBLE;
					else
						m_uiFlags &= ~FLAGS_FORCE_VISIBLE;
				}

				break;
			}
			case 'l':
				if ( TStringManager::String8Compare( szName, "listenanim" ) == 0 )
					m_strListenAnim = pValue->GetTPString8();

				break;
			case 'm':
				if ( TStringManager::String8Compare( szName, "message" ) == 0 && pValue->GetType() == PBPropertyValue::Type::Int )
					m_aiTexts[ m_iNumTexts++ ] = pValue->GetInteger();

				break;
			case 's':
			{
				TPString8 strName( szName );

				if ( strName == TPS8( sfx ) )
				{
					if ( pValue->GetType() == PBPropertyValue::Type::Int )
					{
						m_iSfx = TINT16( pValue->GetInteger() );
						m_uiFlags |= FLAGS_SFX;
					}
				}
				else if ( strName == TPS8( speechtype ) )
				{
					// NOTE: retail passes the value to a RETURNS_ZERO stub (006dc070) and ignores the result
				}

				break;
			}
			case 't':
				if ( TStringManager::String8Compare( szName, "talkanim" ) == 0 )
					m_strTalkAnim = pValue->GetTPString8();

				break;
		}
	}

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bed60
void ASpeechActivity::GetDataSize( TINT* a_pSize )
{
	*a_pSize = 0x250;
}

// $Barnyard: FUNCTION 005bedb0
TBOOL ASpeechActivity::GetText( TWCHAR** a_ppText, TBOOL a_bKeep )
{
	if ( m_iNumTexts > 0 )
	{
		TINT iText   = ARandom::GetSingleton()->m_oRandom.GetInt( m_iNumTexts );
		TINT iTextId = m_aiTexts[ iText ];

		*a_ppText = ( iTextId != -1 ) ? ALocaleManager::Interface()->GetString( iTextId ) : TNULL;

		if ( a_bKeep )
			m_iText = iText;

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005bee10
AInteractionActivity::RESULT ASpeechActivity::OnStart( AInteractionSequence* a_pSequence )
{
	// Hide the target reticles of the milk squirt
	AGameState* pSimState = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) );

	if ( pSimState )
		TSTATICCAST( ASimState, pSimState )->GetMilkReticles().SetActive( TFALSE );

	SpeechData* pData   = TSTATICCAST( SpeechData, a_pSequence->GetActivityData() );
	pData->uiSequenceId = TUINT( -1 );
	pData->iUnk0c       = -1;
	pData->uiState      = SpeechData::STATE_SPEAKING;
	pData->uiUnk09      = 0;

	m_pPlayer       = TNULL;
	m_pPlayerFacial = TNULL;

	for ( auto it = a_pSequence->GetCategoryBegin( AInteractionSequence::CATEGORY_ALL ); it != a_pSequence->GetCategoryEnd( AInteractionSequence::CATEGORY_ALL ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
		{
			m_pPlayer       = pUnit;
			m_pPlayerFacial = TSTATICCAST( AUnitFacialComponent, pUnit->FindComponentExact( &TGetClass( AUnitFacialComponent ) ) );
		}
	}

	return RESULT_DONE;
}

// Normalises the vector on the XZ plane (y = 0), (0, 0, 1) if it's zero
// NOTE: a TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// $Barnyard: FUNCTION 005bfe90
AInteractionActivity::RESULT ASpeechActivity::Update( AInteractionSequence* a_pSequence )
{
	static TFLOAT s_fMaxDistanceSq = TMath::MAXFLOAT; // 0x0078bb58

	if ( !( m_uiFlags & FLAGS_BUBBLE_VISIBLE_FOR_PLAYER ) )
		return RESULT_DONE;

	TVector4 vecPlayerHead = TVector4::VEC_ZERO;

	if ( m_pPlayer )
		AUnitFacialComponent::GetHeadPosition( m_pPlayer, vecPlayerHead );

	SpeechData* pData   = TSTATICCAST( SpeechData, a_pSequence->GetActivityData() );
	pData->uiSequenceId = a_pSequence->GetId();

	if ( !( a_pSequence->GetActivityState().iFlags & AInteractionActivityState::FLAGS_INITIALISED ) )
	{
		// The first update, show the text
		TVector4 vecSpeakerHead = TVector4::VEC_ZERO;
		TVector4 vecSpeakerPos  = TVector4::VEC_ZERO;
		pData->iSpeech          = -1;
		AUnit* pLookAtUnit      = TNULL;

		auto itSpeaker = a_pSequence->GetCategoryBegin( m_eCategory );

		if ( itSpeaker != a_pSequence->GetCategoryEnd( m_eCategory ) )
		{
			AUnit* pSpeaker = itSpeaker->GetOwner();
			TBOOL  bVisible = m_uiFlags & FLAGS_BUBBLE_VISIBLE;

			if ( pSpeaker && pSpeaker->IsPlayer() )
				bVisible = ( m_uiFlags & FLAGS_BUBBLE_VISIBLE_FOR_PLAYER ) != 0;

			if ( !( m_uiFlags & FLAGS_FORCE_NO_ANIM ) )
			{
				PlayAnim( pSpeaker, m_strTalkAnim, TFALSE );

				AUnitFacialComponent* pFacial = TNULL;

				if ( pSpeaker && pSpeaker->IsA( &TGetClass( AVisibleUnit ) ) &&
				     ( pLookAtUnit = pSpeaker, pFacial = TSTATICCAST( AUnitFacialComponent, pSpeaker->FindComponentExact( &TGetClass( AUnitFacialComponent ) ) ) ) )
				{
					// The speaker looks at the player
					if ( pSpeaker != m_pPlayer )
					{
						if ( !pSpeaker->IsA( &TGetClass( AAnimatedUnit ) ) || !TSTATICCAST( AAnimatedUnit, pSpeaker )->GetAnimatableObject()->GetParentObject() )
							pFacial->LookAtUnit( m_pPlayer, vecPlayerHead, -1.0f );
					}

					if ( !AUnitFacialComponent::GetHeadPosition( pSpeaker, vecSpeakerHead ) )
						pLookAtUnit = TNULL;

#ifdef BARNYARD_COMMUNITY_PATCH
					// NOTE: retail dereferences pLookAtUnit even if the speaker has no head bone
					if ( pLookAtUnit )
#endif
					{
						const TVector4* pPos = pLookAtUnit->GetSteer()->GetPosition();
						vecSpeakerPos.x      = pPos->x + vecSpeakerHead.x;
						vecSpeakerPos.y      = pPos->y + vecSpeakerHead.y;
						vecSpeakerPos.z      = pPos->z + vecSpeakerHead.z;
						vecSpeakerPos.w      = pPos->w;
					}
				}
				else
				{
					pLookAtUnit = TNULL;
				}
			}

			pData->iSpeech = ShowSpeech( a_pSequence, pSpeaker, pData->uiSequenceId, TNULL, bVisible, 0 );
		}

		if ( m_eCategory != AInteractionSequence::CATEGORY_ALL )
		{
			// The others listen
			for ( TINT i = AInteractionSequence::CATEGORY_PRIMARY; i <= AInteractionSequence::CATEGORY_AUXILIARY; i++ )
			{
				if ( i == m_eCategory )
					continue;

				for ( auto it = a_pSequence->GetCategoryBegin( AInteractionSequence::CATEGORY( i ) ); it != a_pSequence->GetCategoryEnd( AInteractionSequence::CATEGORY( i ) ); it++ )
				{
					AUnit* pUnit = it->GetOwner();

					if ( !pUnit->IsPlayer() )
					{
						if ( m_strListenAnim == TPS8( idle_lp ) && HasAnim( pUnit, TPS8( idle_aux ) ) )
							m_strListenAnim = TPS8( idle_aux );
						else if ( m_strListenAnim == TPS8( idle_aux ) && !HasAnim( pUnit, TPS8( idle_aux ) ) )
							m_strListenAnim = TPS8( idle_lp );
					}
					else
					{
						m_strListenAnim = TPS8( idle_lp );
					}

					PlayAnim( pUnit, m_strListenAnim, TFALSE );

					if ( pUnit && pUnit->IsA( &TGetClass( AAnimatedUnit ) ) )
					{
						AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, pUnit->FindComponentExact( &TGetClass( AUnitFacialComponent ) ) );

						if ( !TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetParentObject() && pFacial )
						{
							if ( !pLookAtUnit )
							{
								pFacial->StopLook();
							}
							else if ( pUnit != pLookAtUnit )
							{
								// Look at the speaker if it's in front of the listener
								TVector4 vecToSpeaker;
								TVector4 vecHead;

								if ( AUnitFacialComponent::GetHeadPosition( pUnit, vecHead ) )
								{
									const TVector4* pPos = pUnit->GetSteer()->GetPosition();
									vecHead.x += pPos->x;
									vecHead.y += pPos->y;
									vecHead.z += pPos->z;

									vecToSpeaker = TVector4( vecSpeakerPos.x - vecHead.x, vecSpeakerPos.y - vecHead.y, vecSpeakerPos.z - vecHead.z, vecSpeakerPos.w );
									NormaliseXZ( vecToSpeaker );
								}
								else
								{
									const TVector4* pSpeakerPos = pLookAtUnit->GetSteer()->GetPosition();
									const TVector4* pPos        = pUnit->GetSteer()->GetPosition();

									vecToSpeaker = TVector4( pSpeakerPos->x - pPos->x, pSpeakerPos->y - pPos->y, pSpeakerPos->z - pPos->z, pSpeakerPos->w );
									NormaliseXZ( vecToSpeaker );
								}

								if ( TVector4::DotProduct3( *pUnit->GetSteer()->GetDirection(), vecToSpeaker ) > 0.70710677f )
								{
									if ( pLookAtUnit->IsA( &TGetClass( AAnimatedUnit ) ) )
									{
										// Not if the speaker is attached to the listener
										if ( TSTATICCAST( AAnimatedUnit, pLookAtUnit )->GetAnimatableObject()->GetParentObject() != TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject() )
											pFacial->LookAtUnit( pLookAtUnit, vecSpeakerHead, -1.0f );
									}
									else
									{
										pFacial->LookAtUnit( pLookAtUnit, vecSpeakerHead, -1.0f );
									}
								}
							}
						}
					}
				}
			}
		}

		if ( m_uiFlags & FLAGS_SFX )
		{
			AVOStream::GetSingleton()->Play( m_iSfx, TFALSE );
		}

		a_pSequence->GetActivityState().iFlags |= AInteractionActivityState::FLAGS_INITIALISED;
	}

	// The unit the distances are measured from
	AUnit* pFirstUnit = a_pSequence->GetUnit( 0 );

	if ( !pFirstUnit )
	{
		for ( TINT i = AInteractionSequence::CATEGORY_PRIMARY; i <= AInteractionSequence::CATEGORY_AUXILIARY; i++ )
		{
			if ( i == m_eCategory )
				continue;

			for ( auto it = a_pSequence->GetCategoryBegin( AInteractionSequence::CATEGORY( i ) ); it != a_pSequence->GetCategoryEnd( AInteractionSequence::CATEGORY( i ) ); it++ )
			{
				pFirstUnit = it->GetOwner();

				if ( pFirstUnit )
					break;
			}
		}
	}

	// How far the farthest listener is
	TFLOAT fDistanceSq = 0.0f;

	for ( TINT i = AInteractionSequence::CATEGORY_PRIMARY; i <= AInteractionSequence::CATEGORY_AUXILIARY; i++ )
	{
		if ( i == m_eCategory )
			continue;

		for ( auto it = a_pSequence->GetCategoryBegin( AInteractionSequence::CATEGORY( i ) ); it != a_pSequence->GetCategoryEnd( AInteractionSequence::CATEGORY( i ) ); it++ )
		{
			AUnit* pUnit = it->GetOwner();

			if ( pUnit != pFirstUnit )
			{
				const TVector4* pPos      = pUnit->GetSteer()->GetPosition();
				const TVector4* pFirstPos = pFirstUnit->GetSteer()->GetPosition();

				TFLOAT fDistX = pPos->x - pFirstPos->x;
				TFLOAT fDistZ = pPos->z - pFirstPos->z;
				TFLOAT fDist  = fDistZ * fDistZ + fDistX * fDistX;

				if ( fDist > fDistanceSq )
					fDistanceSq = fDist;
			}
		}
	}

	if ( IsSpeakerShown( a_pSequence ) && ASpeechDialogManager::GetSingleton()->IsSpeechFinished( pData->uiSequenceId ) )
	{
		// The text is shown, the speaker stops talking
		auto itSpeaker = a_pSequence->GetCategoryBegin( m_eCategory );

		if ( itSpeaker != a_pSequence->GetCategoryEnd( m_eCategory ) )
		{
			AUnit* pSpeaker = itSpeaker->GetOwner();

			if ( HasAnim( pSpeaker, TPS8( idle_aux ) ) )
				PlayAnim( pSpeaker, TPS8( idle_aux ), TTRUE );
			else
				PlayAnim( pSpeaker, TPS8( idle_lp ), TTRUE );
		}
	}

	if ( pData->uiState == SpeechData::STATE_SPEAKING &&
	     ( pData->uiUnk09 || pData->iSpeech < 0 ||
	       ( !IsSpeakerShown( a_pSequence ) && a_pSequence->GetActivityState().fTime >= 5.0f ) ||
	       ( fDistanceSq >= s_fMaxDistanceSq && a_pSequence->GetActivityState().fTime > 0.01f ) ) )
	{
		// The speech is over (skipped by the player, timed out or the units are too far from each other)
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnit* pUnit = it->GetOwner();

			if ( pUnit )
				PlayIdleAnim( pUnit, pData->uiSequenceId );
		}

		if ( m_eCategory != AInteractionSequence::CATEGORY_ALL )
		{
			for ( TINT i = AInteractionSequence::CATEGORY_PRIMARY; i <= AInteractionSequence::CATEGORY_AUXILIARY; i++ )
			{
				if ( i == m_eCategory )
					continue;

				for ( auto it = a_pSequence->GetCategoryBegin( AInteractionSequence::CATEGORY( i ) ); it != a_pSequence->GetCategoryEnd( AInteractionSequence::CATEGORY( i ) ); it++ )
				{
					AUnit* pUnit = it->GetOwner();
					TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject();

					if ( fDistanceSq < s_fMaxDistanceSq || pUnit->GetSteer()->GetSpeed() <= 0.001f )
					{
						TPString8 strAnim = TPS8( idle_lp );

						if ( HasAnim( pUnit, TPS8( idle_aux ) ) )
							strAnim = TPS8( idle_aux );

						PlayAnim( pUnit, strAnim, TTRUE );
					}
					else
					{
						PlayAnim( pUnit, TPS8( walk_lp ), TTRUE );
					}
				}
			}
		}

		ASpeechDialogManager::GetSingleton()->StopSpeech( pData->uiSequenceId );
		pData->uiState = SpeechData::STATE_STOPPED;
	}

	if ( pData->uiState != SpeechData::STATE_STOPPED )
		return RESULT_RUNNING;

	// Wait until every unit plays its idle animation
	TBOOL bDone = TTRUE;

	for ( auto it = a_pSequence->GetCategoryBegin( AInteractionSequence::CATEGORY_PRIMARY ); it != a_pSequence->GetCategoryEnd( AInteractionSequence::CATEGORY_AUXILIARY ); it++ )
	{
		AUnit*             pUnit             = it->GetOwner();
		AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject();

		if ( ( m_strTalkAnim == TPS8( none ) && ( a_pSequence->GetUnitByCategory( m_eCategory ) == pUnit || m_eCategory == AInteractionSequence::CATEGORY_ALL ) ) ||
		     ( m_strListenAnim == TPS8( none ) && a_pSequence->GetUnitByCategory( m_eCategory ) != pUnit ) )
		{
			continue;
		}

		TPString8 strAnim = TPS8( idle_lp );

		if ( HasAnim( pUnit, TPS8( idle_aux ) ) )
			strAnim = TPS8( idle_aux );

		if ( !( m_uiFlags & FLAGS_FORCE_NO_ANIM ) )
			PlayAnim( pUnit, strAnim, TTRUE );

		if ( HasAnim( pUnit, strAnim ) && !pAnimatableObject->IsAnimPlaying( strAnim, 1.0f ) )
		{
			if ( HasAnim( pUnit, TPS8( walk_lp ) ) )
			{
				if ( !pAnimatableObject->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
					bDone = TFALSE;
			}
			else if ( pUnit->GetSteer()->GetSpeed() <= 0.001f )
			{
				bDone = TFALSE;
			}
		}

		if ( m_uiFlags & FLAGS_FORCE_NO_ANIM )
		{
			// Wait for the sound instead
			bDone = !( m_uiFlags & FLAGS_SFX ) || pData->iUnk0c == -1 || ASoundManager::GetSingleton()->IsStreamCueActive( pData->iUnk0c ) == TFALSE;
		}
	}

	if ( bDone )
	{
		AVOStream::GetSingleton()->Stop();
		return RESULT_DONE;
	}

	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005bf220
TBOOL ASpeechActivity::SkipSpeech( AInteractionSequence* a_pSequence )
{
	SpeechData* pData = TSTATICCAST( SpeechData, a_pSequence->GetActivityData() );

	if ( ASpeechDialogManager::GetSingleton()->SkipSpeech( pData->uiSequenceId ) == TTRUE )
	{
		pData->uiUnk09 = TTRUE;
		AConversationManager::GetSingleton()->OnSequenceEnd( pData->uiSequenceId );
		ASpeechDialogManager::GetSingleton()->StopSpeech( pData->uiSequenceId );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005bfb20
TBOOL ASpeechActivity::PlayAnim( AUnit* a_pUnit, const TPString8& a_rcName, TBOOL a_bForce )
{
	if ( !a_bForce )
	{
		if ( !a_pUnit || !a_pUnit->IsA( &TGetClass( AAnimatedUnit ) ) || a_pUnit->GetSteer()->GetSpeed() >= 0.1f )
			return TFALSE;
	}

	AAnimatableObject*        pAnimatableObject = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();
	AToshiAnimationInterface& rAnimInterface    = pAnimatableObject->GetToshiAnimInterface();
	ANamedAnimation*          pNamedAnimation   = rAnimInterface.GetAnimationSet()->GetNamedAnimation( a_rcName );

	if ( !pNamedAnimation )
		return TFALSE;

	if ( rAnimInterface.IsAnimPlaying( a_rcName, TFALSE ) || pAnimatableObject->IsAnimationPlanned( a_rcName ) )
		return TTRUE;

	TBOOL bPlayed = pAnimatableObject->PlayAnimation( a_rcName );
	return bPlayed || pAnimatableObject->IsAnimationPlanned( a_rcName );
}

// $Barnyard: FUNCTION 005bfc60
TBOOL ASpeechActivity::HasAnim( AUnit* a_pUnit, const TPString8& a_rcName )
{
	if ( !a_pUnit || !a_pUnit->IsA( &TGetClass( AAnimatedUnit ) ) )
		return TFALSE;

	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();
	return pAnimatableObject->GetToshiAnimInterface().GetAnimationSet()->GetNamedAnimation( a_rcName ) != TNULL;
}

// $Barnyard: FUNCTION 005beee0
TBOOL ASpeechActivity::IsSpeakerShown( AInteractionSequence* a_pSequence )
{
	ACameraHelper* pSpeechHelper = ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_SPEECH );

	if ( ACameraManager::GetSingleton()->GetCurrentCameraHelper() != pSpeechHelper )
		return TFALSE;

	TINT iSpeakerId = TSTATICCAST( ASpeechCameraHelper, pSpeechHelper )->GetSpeakerId();

	for ( auto it = a_pSequence->GetComponents().Begin(); it != a_pSequence->GetComponents().End(); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && pUnit->GetId() == iSpeakerId )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005bef70
TWCHAR* ASpeechActivity::FormatText( AInteractionSequence* a_pSequence, TINT a_iText )
{
	SpeechData* pData = TSTATICCAST( SpeechData, a_pSequence->GetActivityData() );

	TWCHAR wszString[ 1024 ];
	ALocale::FormatInputTags( wszString, 1024, m_aiTexts[ a_iText ] != -1 ? ALocaleManager::Interface()->GetString( m_aiTexts[ a_iText ] ) : TNULL );

	const TWCHAR* pSrc       = wszString;
	TWCHAR*       pDst       = pData->wszText;
	TINT          iCharsLeft = TStringManager::String16Length( wszString ) + 1;

	while ( iCharsLeft > 0 )
	{
		TWCHAR wc = *pSrc++;

		if ( wc != '%' )
		{
			*pDst++ = wc;
			iCharsLeft--;
			continue;
		}

		// A tag, "%<unit>.<value>": the unit is "a" (auxiliary), "s" (secondary), "player" or the primary unit
		AUnit* pUnit = TNULL;
		iCharsLeft--;

		switch ( *pSrc )
		{
			case 'a':
				pUnit = a_pSequence->GetUnit( 2 );
				pSrc++;
				iCharsLeft--;
				break;
			case 's':
				pUnit = a_pSequence->GetUnit( 1 );
				pSrc++;
				iCharsLeft--;
				break;
			case 'p':
				if ( TStringManager::String16Compare( pSrc, L"player", 6 ) == 0 )
				{
					pUnit = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
					pSrc += 6;
					iCharsLeft -= 6;
				}
				else
				{
					pUnit = a_pSequence->GetUnit( 0 );
					pSrc++;
					iCharsLeft--;
				}

				break;
		}

		// Skip the separator
		pSrc++;
		iCharsLeft--;

		if ( TStringManager::String16Compare( pSrc, L"name", 4 ) == 0 )
		{
			if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
			{
				const TString16& rcName = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName();
				TStringManager::String16Copy( pDst, rcName.GetString() );
				pDst += rcName.Length();
			}
			else
			{
				const TWCHAR* wszName = pUnit->GetScreenName();
				TStringManager::String16Copy( pDst, wszName );
				pDst += TStringManager::String16Length( wszName );
			}

			pSrc += 4;
			iCharsLeft -= 4;
		}
		else if ( TStringManager::String16Compare( pSrc, L"species", 7 ) == 0 )
		{
			const TWCHAR* wszName = pUnit->GetScreenName();
			TStringManager::String16Copy( pDst, wszName );
			pDst += TStringManager::String16Length( wszName );

			pSrc += 7;
			iCharsLeft -= 7;
		}
		else if ( TStringManager::String16Compare( pSrc, L"nbpc", 4 ) == 0 )
		{
			// The percentage of the gopher shop that has been bought
			TCHAR  szNumber[ 16 ];
			TWCHAR wszNumber[ 16 ];
			T2String8::IntToString( AGopherShopInterface::GetPercentBought(), szNumber, 10 );
			TStringManager::StringCharToUnicode( wszNumber, szNumber, -1 );
			TStringManager::String16Copy( pDst, wszNumber );
			pDst += TStringManager::String16Length( wszNumber );

			pSrc += 4;
			iCharsLeft -= 4;
		}
		else if ( TStringManager::String16Compare( pSrc, L"rank", 4 ) == 0 )
		{
			// The rank of the player in the champion chart with its ordinal suffix ("1st", "2nd"...)
			// NOTE: retail doesn't check the rank, s_aRankSuffixes[ 0 ] isn't a locale id (DAT_0072bfe8)
			static const TINT s_aRankSuffixes[] = { 981668463, 267, 268, 269, 270, 270 };

			TINT   iRank = ASimPersistData::GetSingleton()->GetRank();
			TCHAR  szNumber[ 16 ];
			TWCHAR wszNumber[ 16 ];
			T2String8::IntToString( iRank, szNumber, 10 );
			TStringManager::StringCharToUnicode( wszNumber, szNumber, -1 );
			TStringManager::String16Copy( pDst, wszNumber );
			pDst += TStringManager::String16Length( wszNumber );

			const TWCHAR* wszSuffix = ALocaleManager::Interface()->GetString( s_aRankSuffixes[ iRank ] );
			TStringManager::String16Copy( pDst, wszSuffix );
			pDst += TStringManager::String16Length( wszSuffix );

			pSrc += 4;
			iCharsLeft -= 4;
		}
	}

	return pData->wszText;
}

// $Barnyard: FUNCTION 005bf870
TINT ASpeechActivity::ShowSpeech( AInteractionSequence* a_pSequence, AUnit* a_pUnit, TUINT a_uiId, void* a_pUnused, TBOOL a_bVisible, TINT a_iUnused )
{
	if ( !a_pUnit || m_iNumTexts <= 0 || !a_pUnit->IsA( &TGetClass( AVisibleUnit ) ) )
		return -1;

	TINT iText = ARandom::GetSingleton()->m_oRandom.GetInt( m_iNumTexts );

	if ( m_iText >= 0 )
	{
		// The text picked by GetText
		iText   = m_iText;
		m_iText = -1;
	}

	const TWCHAR* wszText = FormatText( a_pSequence, iText );

	if ( TStringManager::String16Compare( wszText, L"Auto-complete my quest?" ) == 0 ||
	     TStringManager::String16Compare( wszText, L"Commencement Dialogue." ) == 0 ||
	     TStringManager::String16Compare( wszText, L"Intermediate Dialogue." ) == 0 )
	{
		// Placeholder texts of the quests, add the name of the quest
		static TWCHAR s_wszText[ 256 ]; // 0x0078b928

		AUnit*          pUnit     = GetActiveUnit( a_pSequence );
		AQuestInstance* pInstance = TNULL;

		for ( TINT i = 0; i < AQuestManager::GetSingleton()->GetNumAssignedQuests(); i++ )
		{
			AQuestInstance* pAssigned = AQuestManager::GetSingleton()->GetAssignedQuest( i );

			if ( pAssigned->GetQuestGiver() == pUnit )
				pInstance = pAssigned;
		}

		if ( pInstance )
		{
			TWCHAR wszQuestName[ 256 ];
			TStringManager::StringCharToUnicode( wszQuestName, pInstance->GetQuest()->GetName().GetString(), -1 );
			TStringManager::String16Format( s_wszText, 256, L"%s (%s)", wszText, wszQuestName );
			wszText = s_wszText;
		}
	}

	ASpeechCameraHelper* pSpeechHelper = TSTATICCAST( ASpeechCameraHelper, ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_SPEECH ) );

	// The name of the speaker in upper case
	const TWCHAR* wszName = a_pUnit->IsPlayer() ? APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString() : a_pUnit->GetType()->GetScreenName();

	TString16   strName( wszName );
	SpeechData* pData = TSTATICCAST( SpeechData, a_pSequence->GetActivityData() );
	TStringManager::String16Copy( pData->wszName, strName.MakeUpper().GetString() );

	ASpeechDialogManager::GetSingleton()->StartSpeech( pData->wszName, wszText, a_pUnit, a_uiId, !a_bVisible );
	pSpeechHelper->SetSpeaker( -1, m_iCamera, a_pUnit->GetId(), TTRUE );

	return iText;
}

// $Barnyard: FUNCTION 005bfdd0
void ASpeechActivity::PlayIdleAnim( AUnit* a_pUnit, TUINT a_uiId )
{
	if ( !( m_uiFlags & FLAGS_FORCE_NO_ANIM ) )
	{
		TPString8 strAnim = TPS8( idle_lp );

		if ( HasAnim( a_pUnit, TPS8( idle_aux ) ) )
			strAnim = TPS8( idle_aux );

		PlayAnim( a_pUnit, strAnim, TTRUE );
	}
}

// $Barnyard: FUNCTION 005bed70
AUnit* ASpeechActivity::GetTargetUnit( AInteractionSequence* a_pSequence )
{
	// The primary unit listens to the others
	if ( m_eCategory != AInteractionSequence::CATEGORY_PRIMARY )
		return a_pSequence->GetUnit( 0 );

	if ( a_pSequence->GetComponents().Size() > 1 )
		return a_pSequence->GetUnit( 1 );

	return TNULL;
}
