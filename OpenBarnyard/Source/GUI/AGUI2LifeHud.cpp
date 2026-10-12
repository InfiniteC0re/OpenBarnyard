#include "pch.h"
#include "AGUI2LifeHud.h"
#include "ARandom.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "Render/ARenderer.h"
#include "SoundBank/am_f.h"
#include "SoundBank/am_m.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// 00779844
static const TCHAR* s_apDefaultSprites[ AGUI2LifeHud::NUM_DEFAULT_SPRITES ] = {
	"Loading_1",
	"Loading_2",
	"Loading_3",
	"Loading_4",
	"Loading_5",
	"Loading_6",
	"Loading_7",
	"Loading_8"
};

// Number pop curve, set up by the static initialisers 0070fc50 and 0070fc70.
static const TFLOAT s_fPopAngle = TMath::PI * 0.75f;                // 007b5074
static const TFLOAT s_fPopNorm  = 1.0f / TMath::Sin( s_fPopAngle ); // 007b5070

// Returns a_iFrame wrapped into [0, a_iNumFrames).
static TINT WrapFrame( TINT a_iFrame, TINT a_iNumFrames )
{
	TINT iFrame = a_iFrame % a_iNumFrames;
	while ( iFrame < 0 ) iFrame += a_iNumFrames;

	return iFrame;
}

// $Barnyard: FUNCTION 0063d8b0
AGUI2LifeHud::AGUI2LifeHud()
    : m_bCreated( TFALSE )
    , m_iLives( 0 )
    , m_iDisplayedLives( 0 )
    , m_fPopTime( 0.0f )
    , m_ppSprites( s_apDefaultSprites )
{
}

// $Barnyard: FUNCTION 0063dd50
// $Barnyard: FUNCTION 0063d960
AGUI2LifeHud::~AGUI2LifeHud()
{
	if ( m_bCreated )
	{
		m_bCreated = TFALSE;
		RemoveSelf();
		TFree( m_ppSpriteSections );
	}
}

// $Barnyard: FUNCTION 0063dd70
void AGUI2LifeHud::Create( Toshi::T2GUIElement* a_pParent, const TCHAR** a_ppSprites, TINT a_iNumSprites, const TCHAR* a_szBacking, Toshi::T2GUIATTACHMENT a_eAnchor, Toshi::T2GUIATTACHMENT a_ePivot, TFLOAT a_fScale, TBOOL a_bNumberBehind )
{
	m_fScale          = a_fScale;
	m_ppLoopFrames    = TNULL;
	m_ppIntroFrames   = TNULL;
	m_iNumLoopFrames  = 0;
	m_iNumIntroFrames = 0;

	for ( TINT i = 0; i < NUM_GLOWS; i++ )
	{
		m_aGlows[ i ].T2GUIElement::SetDimensions( 1.0f, 1.0f );
		m_aGlows[ i ].Hide();
		AddChildTail( m_aGlows[ i ] );
		m_afGlowScalePhase[ i ] = 1.0f;
	}

	m_afGlowAngle[ 0 ]      = 0.0f;
	m_afGlowAngle[ 1 ]      = TMath::PI * 0.2f;
	m_afGlowAngle[ 2 ]      = TMath::PI * ( 1.0f / 7.0f );
	m_afGlowAlphaPhase[ 0 ] = 0.0f;
	m_afGlowAlphaPhase[ 1 ] = TMath::PI;
	m_afGlowScalePhase[ 0 ] = 0.0f;
	m_afGlowAlphaPhase[ 2 ] = TMath::PI * 0.5f;
	m_afGlowScalePhase[ 1 ] = TMath::PI * ( 1.0f / 3.0f );
	m_bCreated              = TTRUE;
	m_afGlowScalePhase[ 2 ] = TMath::PI * 0.25f;

	if ( a_ppSprites )
	{
		m_ppSprites   = a_ppSprites;
		m_iNumSprites = a_iNumSprites;
	}
	else
	{
		m_iNumSprites = NUM_DEFAULT_SPRITES;
	}

	m_ppSpriteSections = TSTATICCAST( T2GUITextureSection*, TMalloc( m_iNumSprites * sizeof( T2GUITextureSection* ) ) );

	for ( TINT i = 0; i < m_iNumSprites; i++ )
		m_ppSpriteSections[ i ] = T2GUITextureSectionManager::GetTextureSection( m_ppSprites[ i ] );

	m_fSpinTarget = 0.0f;
	m_fSpin       = 0.0f;

	// The wooden backing
	T2GUITextureSection* pBacking = T2GUITextureSectionManager::GetTextureSection( a_szBacking );
	TFLOAT               fHeight  = pBacking->GetHeight();
	TFLOAT               fWidth   = pBacking->GetWidth() * ( 5.0f / 7.0f );
	TFLOAT               fHalfH   = fHeight * 0.5f;

	T2GUIElement::SetDimensions( fWidth, fHeight );
	SetTextureSection( pBacking );
	m_eAnchor = a_eAnchor;
	m_ePivot  = a_ePivot;

	if ( a_ePivot == T2GUIATTACHMENT_BOTTOMRIGHT || a_ePivot == T2GUIATTACHMENT_BOTTOMLEFT || a_ePivot == T2GUIATTACHMENT_BOTTOMCENTER )
		fHalfH = -fHalfH;

	TFLOAT fOffsetX                 = ( a_ePivot == T2GUIATTACHMENT_BOTTOMLEFT || a_ePivot == T2GUIATTACHMENT_MIDDLELEFT || a_ePivot == T2GUIATTACHMENT_TOPLEFT ) ? -fHeight : fHeight;
	m_oTransform.m_vecTranslation.x = fOffsetX * -0.5f;
	m_oTransform.m_vecTranslation.y = fHalfH;

	a_pParent->AddChildTail( *this );

	// The spinning sprite
	// NOTE: retail uses the AGUI2Rectangle::SetTextureSection overload at 006c3b00, which also resizes the rectangle.
	m_oSprite.SetDimensions( m_ppSpriteSections[ 0 ]->GetWidth(), m_ppSpriteSections[ 0 ]->GetHeight() );
	m_oSprite.SetTextureSection( m_ppSpriteSections[ 0 ] );

	T2GUITransform oScale;
	oScale.Set( TVector2( m_fScale, 0.0f ), TVector2( 0.0f, m_fScale ), TVector2( 0.0f, 0.0f ) );
	m_oSprite.GetTransform().PreMultiply( oScale );

	// The number
	m_oNumberContainer.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oNumberContainer.SetDimensions( fHeight, fHeight );
	m_oNumberContainer.GetTransform().m_vecTranslation.x = fWidth * -0.1f;
	m_oNumberContainer.GetTransform().m_vecTranslation.y = fHeight * -0.05f;

	if ( a_bNumberBehind )
	{
		AddChildTail( m_oNumberContainer );
		AddChildTail( m_oSprite );
	}
	else
	{
		AddChildTail( m_oSprite );
		AddChildTail( m_oNumberContainer );
	}

	m_oNumberString.SetFont( T2GUIFontManager::FindFont( "rekord26" ) );
	m_oNumberString.SetColour( 0xFFFFFFFF );
	m_oNumberString.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oNumberString.SetText( m_wszLives );
	m_oNumberString.SetScale( 1.5f );
	m_oNumberContainer.AddChildTail( m_oNumberString );

	m_iLives          = 0;
	m_fSpinTarget     = 0.0f;
	m_fSpin           = 0.0f;
	m_iDisplayedLives = 0;
	TStringManager::String16Format( m_wszLives, 4, L"%d", 0 );
	m_oNumberString.SetText( m_wszLives );
	TStringManager::String16Format( m_wszLives, 4, L"%d", m_iLives );
	m_oNumberString.SetText( m_wszLives );

	m_fWobblePhase   = 0.0f;
	m_fRotation      = 0.0f;
	m_fWobbleTarget  = 0.0f;
	m_fWobble        = 0.0f;
	m_bWobbleSpin    = TFALSE;
	m_fGlowDuration  = 0.0f;
	m_fGlowFadeIn    = 0.0f;
	m_fPulse         = 1.0f;
	m_fSpriteOffsetX = -11.0f;
}

// $Barnyard: FUNCTION 0063ce80
void AGUI2LifeHud::Destroy()
{
	m_bCreated = TFALSE;
	RemoveSelf();
	TFree( m_ppSpriteSections );
}

// $Barnyard: FUNCTION 0063ceb0
void AGUI2LifeHud::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible == TTRUE )
		Show();
	else
		Hide();

	if ( m_ppSpriteSections && m_ppSpriteSections[ 0 ] )
		m_oSprite.SetTextureSection( m_ppSpriteSections[ 0 ] );
}

// $Barnyard: FUNCTION 0063e160
void AGUI2LifeHud::Tick( TFLOAT a_fDeltaTime )
{
	if ( !m_ppLoopFrames && !m_ppIntroFrames )
		UpdateSpin( a_fDeltaTime );
	else
		UpdateFrameAnimation( a_fDeltaTime );

	UpdateNumber( a_fDeltaTime );
	UpdateWobble( a_fDeltaTime );
	UpdateGlows( a_fDeltaTime );

	T2GUITransform& rTransform = m_oSprite.GetTransform();
	rTransform.SetIdentity();
	rTransform.Rotate( m_fRotation );
	rTransform.SetTranslation( m_fSpriteOffsetX, 0.0f );

	TFLOAT fScale = m_fPulse * m_fScale;
	rTransform.Scale( fScale, fScale );

	T2GUIRectangle::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0063d5c0
void AGUI2LifeHud::Render()
{
	// Glows behind the backing
	if ( m_aGlows[ 0 ].IsVisible() )
	{
		for ( TINT i = 0; i < NUM_GLOWS; i++ )
		{
			if ( m_aGlows[ i ].IsVisible() )
			{
				m_aGlows[ i ].PreRender();
				m_aGlows[ i ].Render();
				m_aGlows[ i ].PostRender();
			}
		}
	}

	T2GUIRectangle::Render();

	// And faded copies on top of it while they're fading out
	if ( m_aGlows[ 0 ].IsVisible() )
	{
		TFLOAT fProgress = m_fGlowTimeLeft / m_fGlowDuration;
		TMath::Clip( fProgress, 0.0f, 1.0f );

		for ( TINT i = 0; i < NUM_GLOWS; i++ )
		{
			if ( m_aGlows[ i ].IsVisible() )
			{
				TFLOAT fAlpha = m_aGlows[ i ].GetAlpha();
				m_aGlows[ i ].SetAlpha( fAlpha * fProgress * fProgress * fProgress * 1.25f );
				m_aGlows[ i ].PreRender();
				m_aGlows[ i ].Render();
				m_aGlows[ i ].PostRender();
				m_aGlows[ i ].SetAlpha( fAlpha );
			}
		}
	}
}

// $Barnyard: FUNCTION 0063da20
void AGUI2LifeHud::SetLives( TINT a_iLives )
{
	m_iLives          = a_iLives;
	m_fSpinTarget     = 0.0f;
	m_fSpin           = 0.0f;
	m_iDisplayedLives = a_iLives;
	TStringManager::String16Format( m_wszLives, 4, L"%d", a_iLives );
	m_oNumberString.SetText( m_wszLives );
}

// $Barnyard: FUNCTION 0063e310
void AGUI2LifeHud::AddLives( TINT a_iCount )
{
	if ( a_iCount > 0 )
	{
		for ( TINT i = a_iCount; i != 0; i-- )
			AddLife();
	}
	else if ( a_iCount < 0 )
	{
		for ( TINT i = -a_iCount; i != 0; i-- )
		{
			TINT iNewLives = m_iLives - 1;
			if ( iNewLives < 0 ) iNewLives = 0;

			m_iDisplayedLives = m_iLives;
			m_iLives          = iNewLives;
			m_fSpinTarget     = 0.0f;
			m_fSpin           = 0.0f;
			AddSpin( -SPIN_PER_LIFE );
			m_fPopTime = POP_DURATION;
		}
	}
}

// $Barnyard: FUNCTION 0063e2a0
void AGUI2LifeHud::AddLife()
{
	if ( m_iLives < MAX_LIVES )
	{
		m_iDisplayedLives = m_iLives;
		m_iLives += 1;
		m_fSpinTarget = 0.0f;
		m_fSpin       = 0.0f;
		AddSpin( SPIN_PER_LIFE );
		m_fPopTime = POP_DURATION;
		PlayLifeGainedSounds();

		if ( ASoundManager::GetSingleton()->FindPlayingCue( soundbank::UI_LIFE ) == -1 )
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_LIFE );
	}
}

// $Barnyard: FUNCTION 0063e210
void AGUI2LifeHud::AddLife( const Toshi::TVector4& a_rcScreenPos )
{
	// NOTE: retail converts a copy of a_rcScreenPos to GUI coordinates and doesn't use the result
	TVector4 vecPos = a_rcScreenPos;
	ARenderer::GetSingleton()->WorldToGUI( vecPos );

	if ( m_iLives < MAX_LIVES )
	{
		m_iDisplayedLives = m_iLives;
		m_iLives += 1;
		m_fSpinTarget = 0.0f;
		m_fSpin       = 0.0f;
		AddSpin( SPIN_PER_LIFE );
		m_fPopTime = POP_DURATION;
		PlayLifeGainedSounds();
	}
}

// $Barnyard: FUNCTION 0063da70
void AGUI2LifeHud::PlayLifeGainedSounds()
{
	if ( ASoundManager::GetSingleton()->FindPlayingCue( soundbank::UI_LIFE ) == -1 )
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_LIFE );

	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();

	if ( pProfileManager && pProfileManager->GetMainGender() == APlayerProfile::GENDER_MALE )
		ASoundManager::GetSingleton()->PlayCue( soundbank::AM_M_CHEER );
	else
		ASoundManager::GetSingleton()->PlayCue( soundbank::AM_F_CHEER );
}

// $Barnyard: FUNCTION 0063d420
void AGUI2LifeHud::AddSpin( TFLOAT a_fAmount )
{
	m_fSpinTarget += a_fAmount;

	while ( m_fSpinTarget > MAX_SPIN )
	{
		m_fSpin -= SPIN_PER_LIFE;
		m_fSpinTarget -= SPIN_PER_LIFE;
	}

	while ( m_fSpinTarget < -MAX_SPIN )
	{
		m_fSpin += SPIN_PER_LIFE;
		m_fSpinTarget += SPIN_PER_LIFE;
	}
}

// $Barnyard: FUNCTION 0063d4c0
void AGUI2LifeHud::SetWobble( TFLOAT a_fStrength )
{
	if ( !m_bWobbleSpin )
	{
		m_fWobbleTarget = a_fStrength;
		if ( a_fStrength > MAX_WOBBLE ) m_fWobbleTarget = MAX_WOBBLE;
	}
}

// $Barnyard: FUNCTION 0063d500
void AGUI2LifeHud::StartGlow( const TCHAR* a_szTexture, TFLOAT a_fDuration )
{
	if ( a_szTexture )
	{
		T2GUITextureSection* pSection = T2GUITextureSectionManager::GetTextureSection( a_szTexture );
		m_fGlowTimeLeft               = a_fDuration;

		for ( TINT i = 0; i < NUM_GLOWS; i++ )
		{
			m_aGlows[ i ].Show();
			m_aGlows[ i ].SetTextureSection( pSection );
			m_aGlows[ i ].SetDimensions( pSection->GetWidth(), pSection->GetHeight() );
		}

		m_fGlowDuration = a_fDuration;
		m_fGlowFadeIn   = 0.0f;
	}
	else
	{
		m_aGlows[ 0 ].Hide();
	}
}

// $Barnyard: FUNCTION 0063d7c0
void AGUI2LifeHud::SetFrameAnimations( Toshi::T2GUITextureSection** a_ppLoopFrames, Toshi::T2GUITextureSection** a_ppIntroFrames, TINT a_iNumLoopFrames, TINT a_iNumIntroFrames, TFLOAT a_fLoopSpeed, TFLOAT a_fIntroSpeed )
{
	m_iNumLoopFrames  = a_iNumLoopFrames;
	m_iNumIntroFrames = a_iNumIntroFrames;
	m_fLoopSpeed      = a_fLoopSpeed;
	m_ppLoopFrames    = a_ppLoopFrames;
	m_ppIntroFrames   = a_ppIntroFrames;
	m_fSpin           = 0.0f;
	m_fIntroSpeed     = a_fIntroSpeed;

	if ( !a_ppLoopFrames && !a_ppIntroFrames )
	{
		// Back to the spin animation
		m_fSpinTarget = 0.0f;
		m_oSprite.SetTextureSection( m_ppSpriteSections[ WrapFrame( TINT( m_iNumSprites * 0.0f ), m_iNumSprites ) ] );
	}
}

// $Barnyard: FUNCTION 0063d860
void AGUI2LifeHud::SetFrameAnimation( Toshi::T2GUITextureSection** a_ppFrames, TINT a_iNumFrames, TFLOAT a_fSpeed )
{
	SetFrameAnimations( a_ppFrames, a_ppFrames, a_iNumFrames, a_iNumFrames, a_fSpeed, a_fSpeed );
}

// $Barnyard: FUNCTION 0063d880
TBOOL AGUI2LifeHud::IsPlayingFrameAnimation( Toshi::T2GUITextureSection** a_ppFrames ) const
{
	return m_ppLoopFrames == a_ppFrames || m_ppIntroFrames == a_ppFrames;
}

// $Barnyard: FUNCTION 0063cef0
void AGUI2LifeHud::UpdateNumber( TFLOAT a_fDeltaTime )
{
	if ( m_fPopTime > 0.0f )
	{
		m_fPopTime -= a_fDeltaTime;
		TFLOAT fHalfTime = POP_DURATION * 0.5f;

		if ( m_fPopTime <= fHalfTime )
		{
			// Second half: show the new number and let it pop
			if ( m_iDisplayedLives != m_iLives )
			{
				m_iDisplayedLives = m_iLives;
				TStringManager::String16Format( m_wszLives, 4, L"%d", m_iLives );
				m_oNumberString.SetText( m_wszLives );
			}

			if ( m_fPopTime < 0.0f )
				m_oNumberString.SetScale( 1.5f );
			else
				m_oNumberString.SetScale( TMath::Sin( ( ( POP_DURATION * 0.5f - m_fPopTime ) / ( POP_DURATION * 0.5f ) ) * s_fPopAngle ) * s_fPopNorm * 1.5f );
		}
		else
		{
			// First half: shrink the old number
			m_oNumberString.SetScale( ( ( m_fPopTime - fHalfTime ) / fHalfTime ) * 1.5f );
		}
	}
}

// $Barnyard: FUNCTION 0063d000
void AGUI2LifeHud::UpdateSpriteFrame( TBOOL a_bPulse )
{
	m_oSprite.SetTextureSection( m_ppSpriteSections[ WrapFrame( TINT( m_iNumSprites * m_fSpin ), m_iNumSprites ) ] );

	if ( a_bPulse )
	{
		TFLOAT fAngle = fmodf( TMath::PI * m_fSpin * ( 1.0f / 3.0f ), TMath::PI );
		while ( fAngle < 0.0f ) fAngle += TMath::PI;

		m_fPulse = TMath::Sin( fAngle ) * 0.5f + 1.0f;
	}
}

// $Barnyard: FUNCTION 0063d0a0
void AGUI2LifeHud::UpdateSpin( TFLOAT a_fDeltaTime )
{
	if ( m_fSpin < m_fSpinTarget )
	{
		// Gaining lives
		m_fSpin += ( TMath::Sqrt( TMath::Abs( m_fSpinTarget - m_fSpin ) ) * 1.5f + 0.5f ) * a_fDeltaTime;
		if ( m_fSpin > m_fSpinTarget ) m_fSpin = m_fSpinTarget;

		UpdateSpriteFrame( TTRUE );
		m_oSprite.SetColour( 0xFFFFFFFF );
	}
	else if ( m_fSpinTarget < m_fSpin )
	{
		// Losing lives: the sprite flashes red
		m_fSpin -= ( TMath::Sqrt( TMath::Abs( m_fSpinTarget - m_fSpin ) ) * 1.5f + 0.5f ) * a_fDeltaTime;
		if ( m_fSpin < m_fSpinTarget ) m_fSpin = m_fSpinTarget;

		UpdateSpriteFrame( m_oSprite.GetTransform().GetScaleX() / m_fScale > 1.0f );

		TFLOAT fAngle = TFLOAT( fmod( m_fSpin * ( 1.0f / 3.0f ), 1.0 ) ) * TMath::PI;
		while ( fAngle < 0.0f ) fAngle += TMath::PI;

		TFLOAT fSin = TMath::Sin( fAngle );
		TFLOAT fInv = ( 1.0f - fSin ) * 255.0f;

		TUINT8 uiRed   = TUINT8( TINT( fSin * 255.0f + fInv ) );
		TUINT8 uiGreen = TUINT8( TINT( fSin * 5.0f + fInv ) );
		TUINT8 uiBlue  = TUINT8( TINT( fSin * 0.0f + fInv ) );

		m_oSprite.SetColour( 0xFF000000 | ( uiRed << 16 ) | ( uiGreen << 8 ) | uiBlue );
	}
	else
	{
		m_oSprite.SetColour( 0xFFFFFFFF );
	}
}

// $Barnyard: FUNCTION 0063d270
void AGUI2LifeHud::UpdateWobble( TFLOAT a_fDeltaTime )
{
	if ( m_fWobbleTarget > m_fWobble )
	{
		m_fWobble += a_fDeltaTime;
		if ( m_fWobble > m_fWobbleTarget ) m_fWobble = m_fWobbleTarget;
	}
	else if ( m_fWobbleTarget < m_fWobble )
	{
		m_fWobble -= a_fDeltaTime;
		if ( m_fWobble < m_fWobbleTarget ) m_fWobble = m_fWobbleTarget;
	}

	if ( !m_bWobbleSpin )
	{
		TFLOAT fSpeed = m_fWobble;
		if ( fSpeed > 1.0f ) fSpeed = 1.0f;

		m_fWobblePhase += fSpeed * a_fDeltaTime * 4.0f;
		m_fRotation = TMath::Sin( m_fWobblePhase ) * m_fWobble * m_fWobble * 0.5f;
	}
	else
	{
		m_fRotation += m_fWobble * 0.25f;

		if ( m_fWobble == m_fWobbleTarget )
		{
			m_bWobbleSpin   = TFALSE;
			m_fWobble       = m_fWobble * 0.5f;
			m_fWobbleTarget = 0.0f;

			static TFLOAT s_fHalfPi = TMath::ASin( 1.0f );
			m_fWobblePhase          = s_fHalfPi;
		}

		if ( m_fRotation >= TMath::PI )
			m_fRotation = TMath::PI;
	}
}

// $Barnyard: FUNCTION 0063d6e0
void AGUI2LifeHud::UpdateFrameAnimation( TFLOAT a_fDeltaTime )
{
	m_fSpin += ( m_ppIntroFrames ? m_fIntroSpeed : m_fLoopSpeed ) * a_fDeltaTime;

	// The intro plays once
	if ( m_fSpin >= 1.0f )
		m_ppIntroFrames = TNULL;

	if ( m_ppIntroFrames )
	{
		m_oSprite.SetTextureSection( m_ppIntroFrames[ WrapFrame( TINT( m_iNumIntroFrames * m_fSpin ), m_iNumIntroFrames ) ] );
	}
	else if ( m_ppLoopFrames )
	{
		m_oSprite.SetTextureSection( m_ppLoopFrames[ WrapFrame( TINT( m_iNumLoopFrames * m_fSpin ), m_iNumLoopFrames ) ] );
	}
}

// $Barnyard: FUNCTION 0063dad0
void AGUI2LifeHud::UpdateGlows( TFLOAT a_fDeltaTime )
{
	if ( !m_aGlows[ 0 ].IsVisible() )
		return;

	// Fade in along a sine curve that overshoots a bit
	const TFLOAT fFadeInAngle = TMath::PI * 0.75f;
	m_fGlowFadeIn += a_fDeltaTime * 3.0f;
	TMath::Clip( m_fGlowFadeIn, 0.0f, 1.0f );
	TFLOAT fFadeIn = ( 1.0f / TMath::Sin( fFadeInAngle ) ) * TMath::Sin( fFadeInAngle * m_fGlowFadeIn );

	m_fGlowTimeLeft -= a_fDeltaTime;

	TFLOAT fAlphaSpeed = a_fDeltaTime * 0.3f;
	m_afGlowAngle[ 0 ] += a_fDeltaTime * 0.3f;
	m_afGlowAngle[ 1 ] -= a_fDeltaTime * 0.4f;
	m_afGlowAngle[ 2 ] -= a_fDeltaTime * 0.2f;

	TFLOAT fFadeOut = m_fGlowTimeLeft / m_fGlowDuration + 0.5f;
	if ( fFadeOut >= 1.0f ) fFadeOut = 1.0f;

	for ( TINT i = 0; i < NUM_GLOWS; i++ )
	{
		m_afGlowAlphaPhase[ i ] += fAlphaSpeed;
		TFLOAT fAlpha = ( TMath::Abs( TMath::Sin( m_afGlowAlphaPhase[ i ] ) ) + 1.0f ) * 0.5f;
		TMath::Clip( fAlpha, 0.2f, 0.4f );

		T2GUITransform& rTransform = m_aGlows[ i ].GetTransform();
		rTransform.SetIdentity();
		rTransform.SetTranslation( m_fSpriteOffsetX, 0.0f );
		rTransform.Rotate( m_afGlowAngle[ i ] );

		m_afGlowScalePhase[ i ] += ( ( ARandom::GetSingleton()->m_oRandom.RandRaw() >> 1 ) * 4.6566129e-11f + 0.4f ) * a_fDeltaTime;
		TFLOAT fScale = ( TMath::Abs( TMath::Sin( m_afGlowScalePhase[ i ] ) ) * 0.1f + 0.9f ) * fFadeOut * fFadeIn;
		TMath::Clip( fScale, 0.0f, 1.0f );
		rTransform.Scale( fScale, fScale );

		m_aGlows[ i ].SetAlpha( fAlpha );
	}
}
