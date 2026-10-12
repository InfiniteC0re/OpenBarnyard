#include "pch.h"
#include <Render/TCameraObject.h>
#include "AMilkCanMicroGame.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "FX/AAniEffectManager.h"
#include "FX/ADieAniEffect.h"
#include "FX/AFadeAniEffect.h"
#include "GameInterface/AGameStateController.h"
#include "GUI/AGUI2.h"
#include "HUD/AHudManager.h"
#include "HUD/AInteractionHudElement.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AUnitManager.h"
#include "Units/AVisibleUnit.h"
#include "UnitMovement/Steering/ACommonSteer.h"
#include "Helpers/ARegrowthManager.h"

#include <GUI/T2GUIFontManager.h>
#include "SoundBank/ui.h"

#ifdef PI
#  undef PI
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00532800, CreateObject 00533b40, CreateObjectInPlace 00533ba0
TDEFINE_CLASS( AMilkCanMicroGame );

TPSTRING8_DECLARE( ABarnExploreMinigameState );

TFLOAT AMilkCanMicroGame::ms_fScreenWidth = 0.0f;

// PTR_s_putaway_00773ed0
static const TCHAR* s_aszStrings[ AMilkCanMicroGame::STRING_NUMOF ] = {
	"putaway",
	"NormalBarnyard",
	"milkcarton",
	"milkcan",
	"butter",
	"cream",
	"Hand",
	"urn_fill",
	"urn_pickup",
	"urn_shakestart",
	"urn_shakeloop",
	"urn_shakeend",
	"urn_putdown",
	"urn_holdidle",
};

// Locale strings
static constexpr TINT LOCALE_CHURN_AGAIN    = 162; // after the first press
static constexpr TINT LOCALE_CHURN_MILK     = 163;
static constexpr TINT LOCALE_CHURN_CREAM    = 164;
static constexpr TINT LOCALE_PUT_DOWN_MILK  = 159;
static constexpr TINT LOCALE_PUT_DOWN_CREAM = 160;
static constexpr TINT LOCALE_FAILED_MILK    = 315;
static constexpr TINT LOCALE_FAILED_CREAM   = 316;
static constexpr TINT LOCALE_MADE_BUTTER    = 317;
static constexpr TINT LOCALE_FILLED         = 318;
static constexpr TINT LOCALE_MADE_CREAM     = 319;

// $Barnyard: FUNCTION 00532de0
AMilkCanMicroGame::AMilkCanMicroGame()
    : m_bCollect( TFALSE )
    , m_eProduct( PRODUCT_MILK )
    , m_eState( STATE_START )
    , m_fChurnTime( 0.25f )
    , m_fPower( 0.0f )
    , m_bInteract( TFALSE )
    , m_bUnk5AA( TTRUE )
    , m_pFont( TNULL )
    , m_bTextFading( TFALSE )
    , m_fCameraTime( -1.0f )
    , m_bCameraTurning( TFALSE )
    , m_bTextWasVisible( TFALSE )
{
	for ( TINT i = 0; i < STRING_NUMOF; i++ )
		m_aStrings[ i ] = TPString8( s_aszStrings[ i ] );

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( m_aStrings[ STRING_NormalBarnyard ] );
}

// $Barnyard: FUNCTION 00533c00
// $Barnyard: FUNCTION 00533be0
AMilkCanMicroGame::~AMilkCanMicroGame()
{
}

// $Barnyard: FUNCTION 00532940
TBOOL AMilkCanMicroGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( !m_pCan )
		return TFALSE;

	// The camera can be turned while churning or choosing
	if ( m_eProduct != PRODUCT_MILK || m_eState == STATE_CHOOSING )
		ACameraManager::GetSingleton()->GetCurrentCameraHelper()->ProcessInputCommand( (void*)a_eInputCommand, (void*)a_pInputEvent );

	switch ( a_eInputCommand )
	{
		case AInputCommand_Quit:
			AGameStateController::GetSingleton()->StartMenuIfPossible();
			return TTRUE;
		case AInputCommand_Interact:
			m_bInteract = TTRUE;
			return TTRUE;
		case AInputCommand_Jump:
			m_bJump = TTRUE;
			return TTRUE;
		default:
			return TFALSE; // RETURNS_FALSE 006528c0
	}
}

// $Barnyard: FUNCTION 005329f0
void AMilkCanMicroGame::OnInsertion()
{
	AGameState::OnInsertion();

	TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) )->SetPathPaused( TTRUE );
	m_pInteractionHud = AHudManager::GetSingleton()->FindElement<AInteractionHudElement>();
	ms_fScreenWidth   = AGUI2::GetRootElement()->GetWidth();

	m_pPlayer  = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	m_eState   = STATE_START;
	m_eProduct = PRODUCT_MILK;

	// Finds the can in front of the player
	ASteer*  pSteer = m_pPlayer->GetSteer();
	TVector2 vecDirection( pSteer->GetDirection()->x, pSteer->GetDirection()->z );
	TINT     iType;

	m_pCan = ARegrowthManager::GetSingleton()->FindObjectInFront( *pSteer->GetPosition(), vecDirection, ARegrowthManager::TypeMask::FromName( m_aStrings[ STRING_milkcan ] ), iType, 1.7f, 0.8f );

	if ( m_pCan )
	{
		ARegrowthManager::GrowthObject* pCan = TSTATICCAST( ARegrowthManager::GrowthObject, m_pCan );

		// Faces the can
		m_pSteer                  = m_pPlayer->GetSteer();
		const TVector4* pPosition = m_pSteer->GetPosition();
		m_vecDirection.x          = pCan->vecPosition.x - pPosition->x;
		m_vecDirection.y          = pCan->vecPosition.y - pPosition->y;
		m_vecDirection.z          = pCan->vecPosition.z - pPosition->z;
		m_vecDirection.w          = pCan->vecPosition.w;
		m_vecDirection.Normalise();
		TSTATICCAST( ACommonSteer, m_pSteer )->StopAndFace( m_vecDirection );

		m_iRegrowthType = iType;
		m_pInteractionHud->SetText( TNULL, 2, 0, -1, 0, -1, -1.0f );
		m_fUnkC8 = -1.0f;
		m_HUDParams.SetFlags( AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK15 );

		// The time left of a churn
		AHudManager::GetSingleton()->AddElement( &m_oTimeLeftHud, TFALSE, TTRUE, TFALSE );
		m_oTimeLeftHud.SetForceHide( TTRUE );
		m_oTimeLeftHud.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
		m_oTimeLeftHud.Setup( 5.0f );
		m_oTimeLeftHud.SetTime( 5.0f );

		// The power of a churn
		AHudManager::GetSingleton()->AddElement( &m_oPowerBar, TFALSE, TTRUE, TFALSE );
		m_oPowerBar.Show();
		m_oPowerBar.m_oBar.Hide();
		m_oPowerBar.m_fTargetValue = 0.0f;
		m_oPowerBar.m_fValue       = 0.0f;
		m_oPowerBar.SetBracketPos( 1.0f );
		m_oPowerBar.SetForceHide( TFALSE );

		// The messages
		m_pFont = T2GUIFontManager::FindFont( "rekord26" );
		m_oText.Create( m_pFont, AGUI2::GetRootElement()->GetWidth() * 0.2f );
		m_oText.SetTransform( 0.0f, m_pFont->GetLineHeight() * -2.0f, 0.0f );
		m_oText.SetShadow( TTRUE, 0xa0000000 );
		AGUI2::GetRootElement()->AddChildTail( m_oText );
	}
}

// $Barnyard: FUNCTION 00532c70
void AMilkCanMicroGame::OnRemoval()
{
	TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) )->SetPathPaused( TFALSE );

	if ( m_pCan )
	{
		T2GUIFontManager::ReleaseFont( m_pFont );
		m_oText.RemoveSelf();
		m_pInteractionHud->SetText( TNULL, 2, 0, -1, 0, -1, ms_fScreenWidth );
		m_pInteractionHud->SetText( TNULL, 2, 0, -1, 1, -1, ms_fScreenWidth );
		TSTATICCAST( ACommonSteer, m_pPlayer->GetSteer() )->SetDesiredVelocity( TVector4::VEC_ZERO );
		m_oPowerBar.Detach();
		m_oTimeLeftHud.Detach();
	}

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 005328a0
void AMilkCanMicroGame::OnActivate()
{
	TBOOL bHidePowerBar = TTRUE;

	if ( m_eProduct != PRODUCT_MILK && ( m_eState == STATE_CHURNING || m_eState == STATE_CHURN_START || m_eState == STATE_CHURN_WAIT ) )
		bHidePowerBar = TFALSE;

	m_oPowerBar.SetForceHide( bHidePowerBar );
	m_pInteractionHud->SetForceHide( TFALSE );
	m_oText.SetVisible( m_bTextWasVisible == TTRUE );
}

// $Barnyard: FUNCTION 00532900
void AMilkCanMicroGame::OnDeactivate()
{
	m_oPowerBar.SetForceHide( TTRUE );
	m_pInteractionHud->SetForceHide( TTRUE );
	m_bTextWasVisible = m_oText.IsVisible();
	m_oText.Hide();
}

// $Barnyard: FUNCTION 00532810
void AMilkCanMicroGame::ShowText( TINT a_iString )
{
	if ( a_iString == -1 )
	{
		m_bTextFading = TTRUE;
	}
	else
	{
		const TWCHAR* wszText = g_pLocaleManager->GetString( a_iString );
		TFLOAT        fWidth  = m_pFont->GetTextWidth( wszText, 1.0f );

		m_oText.Show();
		m_oText.SetWidth( fWidth );
		m_oText.SetText( wszText );
		m_oText.SetAlpha( 1.0f );
		m_oText.SetShadowAlpha( 1.0f );
		m_bTextFading = TFALSE;
	}
}

// $Barnyard: FUNCTION 00532d40
void AMilkCanMicroGame::CollectProduct( const TPString8& a_rcType, const TVector4& a_rcPosition )
{
	static const TVector4 s_vecDirection( 0.0f, 0.0f, 1.0f, 1.0f ); // 0x0077d004

	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );

	// NOTE: the result isn't used
	m_pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) );

	AUnit* pUnit = AUnitManager::GetSingleton()->SpawnUnit( a_rcType, a_rcPosition, s_vecDirection );
	TSTATICCAST( AVisibleUnit, pUnit )->GetModelInstance()->SetTransform( a_rcPosition, s_vecDirection );

	AInventoryGroupComponent::CollectUnit( pUnit, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) ) );
}

// $Barnyard: FUNCTION 00533870
TBOOL AMilkCanMicroGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_pCan )
	{
		// There's no can in front of the player
		Remove();
		return AGameState::OnUpdate( a_fDeltaTime );
	}

	TSTATICCAST( ACommonSteer, m_pSteer )->StopAndFace( m_vecDirection );

	if ( !m_pSteer->IsFacingDesired( 0.99f ) )
		return AGameState::OnUpdate( a_fDeltaTime );

	if ( !m_bCameraTurning && m_fCameraTime < 0.0f )
	{
		// Turns the camera behind the player
		ACameraHelper* pHelper   = ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 );
		TVector4       vecPlayer = *m_pSteer->GetPosition();

		TVector4 vecDirection = *m_pSteer->GetDirection();
		vecDirection.Normalise();

		TVector4 vecToPlayer;
		vecToPlayer.Substract( vecPlayer, pHelper->GetCamera()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) );
		vecToPlayer.y  = 0.0f;
		vecDirection.y = 0.0f;

		TVector4 vecCross;
		vecCross.CrossProduct( vecToPlayer, vecDirection );

		m_fCameraSign    = ( vecCross.y <= 0.0f ) ? -1.0f : 1.0f;
		TFLOAT fAngle    = vecToPlayer.GetAngleBetweenVector( vecDirection );
		m_fCameraTime    = 0.0f;
		m_fTime          = 0.0f;
		m_fCameraAngle   = fAngle * 57.295776f;
		m_bCameraTurning = TTRUE;
	}

	AGTAStyleCameraHelper* pCameraHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );

	if ( m_bCameraTurning )
	{
		TFLOAT fYaw   = TFLOAT( sin( 3.1415927f * m_fCameraTime ) * 1.5f * m_fCameraAngle * a_fDeltaTime );
		m_fCameraTime = a_fDeltaTime + m_fCameraTime;

		if ( m_fCameraTime > 1.0f )
			m_bCameraTurning = TFALSE;

		pCameraHelper->SetYaw( fYaw * m_fCameraSign );
		pCameraHelper->SetForceUpdate( TTRUE );
	}

	UpdateGame( a_fDeltaTime );

	if ( m_bTextFading == TTRUE )
	{
		TFLOAT fAlpha = TFLOAT( m_oText.GetColour() >> 24 ) * 0.0039215689f - a_fDeltaTime;

		if ( fAlpha >= 0.0f )
		{
			m_oText.SetAlpha( fAlpha );
			m_oText.SetShadowAlpha( fAlpha );
		}
		else
		{
			m_bTextFading = TFALSE;
			m_oText.Hide();
		}
	}

	if ( m_eState == STATE_FINISHED )
	{
		Remove();
		return TTRUE;
	}

	m_bInteract = TFALSE;
	m_bJump     = TFALSE;

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00532f80
void AMilkCanMicroGame::UpdateGame( TFLOAT a_fDeltaTime )
{
	AAnimatableObject* pAnimObject = TSTATICCAST( AAnimatedUnit, m_pPlayer )->GetAnimatableObject();

	// NOTE: the result isn't used
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, m_pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	pInventory->GetCategory( m_aStrings[ STRING_Hand ] );

	ARegrowthManager::GrowthObject* pCan = TSTATICCAST( ARegrowthManager::GrowthObject, m_pCan );

	switch ( m_eState )
	{
		case STATE_START:
			pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_fill ] );
			m_eState = STATE_FILLING;
			break;
		case STATE_FILLING:
			if ( pAnimObject->IsAnimationFinished( m_aStrings[ STRING_urn_fill ] ) )
			{
				ShowText( LOCALE_FILLED );
				pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_pickup ] );
				m_strProduct = m_aStrings[ STRING_milkcarton ];
				m_eState     = STATE_PICKING_UP;
			}
			break;
		case STATE_PUT_DOWN:
			pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_putdown ] );
			m_eState = STATE_PUTTING_DOWN;
			break;
		case STATE_PICKING_UP:
			if ( pAnimObject->GetAnimationTime( m_aStrings[ STRING_urn_pickup ] ) > 0.4f )
			{
				// The can is in the hands now
				ARegrowthManager::GetSingleton()->TakeObject( m_iRegrowthType, pCan );
				m_eState = STATE_HOLDING;
			}
			break;
		case STATE_HOLDING:
			pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_holdidle ] );
			m_eState                   = STATE_HOLD_WAIT;
			m_fTime                    = 0.0f;
			m_fPower                   = 0.0f;
			m_oPowerBar.m_fTargetValue = 0.0f;
			break;
		case STATE_UNUSED_8:
			if ( pAnimObject->IsAnimationFinished( m_aStrings[ STRING_urn_pickup ] ) )
				m_eState = STATE_HOLDING;
			break;
		case STATE_PUTTING_DOWN:
			if ( pAnimObject->GetAnimationTime( m_aStrings[ STRING_urn_putdown ] ) > 0.8f )
			{
				// The can is back
				ARegrowthManager::GetSingleton()->StartGrowing( m_iRegrowthType, pCan, 0.0f );
				m_eState      = STATE_PUT_DOWN_WAIT;
				m_bTextFading = TTRUE;
			}
			break;
		case STATE_PUT_DOWN_WAIT:
			if ( pAnimObject->IsAnimationFinished( m_aStrings[ STRING_urn_putdown ] ) )
			{
				TVector4 vecPosition = pCan->vecPosition;

				if ( m_bCollect == TTRUE )
					CollectProduct( m_strProduct, vecPosition );

				TMatrix44 matRotation;
				matRotation.SetFromQuaternion( pCan->quatRotation );

				TVector4 vecDirection;
				vecDirection.x = -matRotation.m_f21;
				vecDirection.y = -matRotation.m_f22;
				vecDirection.z = -matRotation.m_f23;
				vecDirection.w = 1.0f;

				// Replaces the can with a unit that fades out
				ARegrowthManager::GetSingleton()->TakeObject( m_iRegrowthType, pCan );
				AUnit* pUnit = AUnitManager::GetSingleton()->SpawnUnit( m_aStrings[ STRING_milkcan ], vecPosition, vecDirection );

				TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetModelInstance()->SetTransform( pCan->vecPosition, pCan->quatRotation );
				TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetModelInstance()->DisableDrawWithDefaultMaterials();

				AFadeAniEffect* pFade = AAniEffectManager::GetSingleton()->CreateEffect<AFadeAniEffect>();
				ADieAniEffect*  pDie  = AAniEffectManager::GetSingleton()->CreateEffect<ADieAniEffect>();
				pDie->SetUnit( pUnit );
				pFade->SetUnit( pUnit );
				pFade->SetSpeed( 0.5f );
				pFade->SetNextEffect( pDie );

				m_eState      = STATE_FINISHED;
				m_bTextFading = TTRUE;
			}
			break;
		case STATE_CHOOSING:
			if ( m_bInteract == TTRUE )
			{
				// Churns
				m_pInteractionHud->SetText( g_pLocaleManager->GetString( LOCALE_CHURN_AGAIN ), 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, -1, ms_fScreenWidth );
				m_pInteractionHud->SetText( TNULL, 2, 0, -1, 1, -1, ms_fScreenWidth );

				m_fTime                    = 5.0f;
				m_fPower                   = 0.0f;
				m_oPowerBar.m_fTargetValue = 0.0f;
				m_oPowerBar.m_fValue       = 0.0f;
				m_oPowerBar.m_oBar.Show();
				m_oPowerBar.SetForceHide( TFALSE );
				m_oTimeLeftHud.SetForceHide( TFALSE );
				m_oTimeLeftHud.SetTime( m_fTime );

				if ( m_eProduct == PRODUCT_MILK )
				{
					m_eProduct   = PRODUCT_CREAM;
					m_strProduct = m_aStrings[ STRING_cream ];
				}
				else if ( m_eProduct == PRODUCT_CREAM )
				{
					m_eProduct   = PRODUCT_BUTTER;
					m_strProduct = m_aStrings[ STRING_butter ];
				}

				m_eState = STATE_CHURN_WAIT;
			}
			else if ( m_bJump == TTRUE )
			{
				// Puts the can down, keeps the product
				m_bCollect = TTRUE;
				pAnimObject->FinishAnimation( m_aStrings[ STRING_urn_holdidle ] );
				m_eState = STATE_PUT_DOWN;
			}
			break;
		case STATE_HOLD_WAIT:
			m_fTime = a_fDeltaTime + m_fTime;

			if ( m_fTime > 2.0f )
			{
				TINT iChurnString   = 0;
				TINT iPutDownString = 0;
				m_eState            = STATE_CHOOSING;
				m_bTextFading       = TTRUE;

				if ( m_eProduct == PRODUCT_MILK )
				{
					iChurnString   = LOCALE_CHURN_MILK;
					iPutDownString = LOCALE_PUT_DOWN_MILK;
				}
				else if ( m_eProduct == PRODUCT_CREAM )
				{
					iChurnString   = LOCALE_CHURN_CREAM;
					iPutDownString = LOCALE_PUT_DOWN_CREAM;
				}

				m_pInteractionHud->SetText( g_pLocaleManager->GetString( iChurnString ), 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, -1, ms_fScreenWidth );
				m_pInteractionHud->SetText( g_pLocaleManager->GetString( iPutDownString ), 2, TINT( m_pInputMap ), AInputCommand_Jump, 1, -1, ms_fScreenWidth );
			}
			break;
		case STATE_CHURN_WAIT:
			if ( m_bInteract == TTRUE && !pAnimObject->IsAnimPlaying( m_aStrings[ STRING_urn_shakestart ], 0.0f ) )
			{
				pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_shakestart ] );
				m_eState     = STATE_CHURN_START;
				m_fChurnTime = 0.25f;
			}
			break;
		case STATE_CHURN_START:
			if ( pAnimObject->IsAnimationFinished( m_aStrings[ STRING_urn_shakestart ] ) )
				m_eState = STATE_CHURNING;
			break;
		case STATE_CHURNING:
			if ( m_fTime < 0.0f || m_fPower > 0.95f )
			{
				if ( m_fPower > 0.95f )
				{
					m_oPowerBar.m_fTargetValue = 1.0f;
					m_fPower                   = 1.0f;
				}

				m_eState = STATE_CHURN_END;
				pAnimObject->FinishAnimation( m_aStrings[ STRING_urn_shakeloop ] );
				pAnimObject->FinishAnimation( m_aStrings[ STRING_urn_holdidle ] );
				pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_shakeend ] );
			}
			break;
		case STATE_CHURN_END:
			if ( pAnimObject->IsAnimationFinished( m_aStrings[ STRING_urn_shakeend ] ) )
			{
				m_pInteractionHud->SetText( TNULL, 2, 0, -1, 0, -1, -1.0f );
				m_oPowerBar.SetForceHide( TTRUE );
				m_oTimeLeftHud.SetForceHide( TTRUE );

				if ( m_fPower >= 0.95f )
				{
					// Churned
					m_bCollect = TTRUE;
					m_eState   = STATE_HOLDING;

					if ( m_eProduct == PRODUCT_CREAM )
					{
						// Can be churned once more
						ShowText( LOCALE_MADE_CREAM );
					}
					else
					{
						ShowText( LOCALE_MADE_BUTTER );
						m_eState = STATE_PUT_DOWN;
					}
				}
				else
				{
					// Failed, the product is lost
					m_bCollect = TFALSE;
					m_eState   = STATE_PUT_DOWN;
					pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_putdown ] );
					ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );
					ShowText( ( m_eProduct == PRODUCT_CREAM ) ? LOCALE_FAILED_CREAM : LOCALE_FAILED_MILK );
				}
			}
			break;
		default:
			break;
	}

	if ( m_eState == STATE_CHURNING || m_eState == STATE_CHURN_START )
	{
		if ( m_bInteract != TTRUE )
		{
			// Loses power while the button isn't pressed
			m_fPower = TMath::Max( m_fPower - a_fDeltaTime * 0.5f, 0.0f );
			m_fChurnTime -= a_fDeltaTime;

			if ( m_fChurnTime < 0.0f )
			{
				if ( m_eState == STATE_CHURN_START )
					pAnimObject->FinishAnimation( m_aStrings[ STRING_urn_shakestart ] );
				else
					pAnimObject->FinishAnimation( m_aStrings[ STRING_urn_shakeloop ] );

				pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_holdidle ] );
			}
		}
		else
		{
			// Each press adds power, butter needs more presses
			if ( m_eProduct == PRODUCT_CREAM )
				m_fPower = m_fPower + 0.2f;
			else if ( m_eProduct == PRODUCT_BUTTER )
				m_fPower = m_fPower + 0.13f;

			m_fPower     = TMath::Min( m_fPower, 1.0f );
			m_fChurnTime = 0.25f;

			if ( !pAnimObject->IsAnimPlaying( m_aStrings[ STRING_urn_shakeloop ], 0.0f ) && m_eState != STATE_CHURN_START )
			{
				pAnimObject->FinishAnimation( m_aStrings[ STRING_urn_holdidle ] );
				pAnimObject->PlayAnimation( m_aStrings[ STRING_urn_shakeloop ] );
			}
		}

		m_oPowerBar.m_fTargetValue = m_fPower;
		m_fTime -= a_fDeltaTime;
		m_oTimeLeftHud.SetTime( m_fTime );
	}
}
