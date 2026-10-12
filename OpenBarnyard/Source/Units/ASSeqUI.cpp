#include "pch.h"
#include "ASSeqUI.h"
#include "AUnitManager.h"
#include "AUnitActor.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "GameInterface/AGameStateController.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUIFontManager.h"
#include <Input/TInputDeviceMouse.h>
#include "HUD/AHudManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AVOStream.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0062b6d0
// NOTE: GetClass 0062b6d0, CreateObject 0062df00, CreateObjectInPlace 0062df60
TDEFINE_CLASS_NORUNTIME( ASSeqUI );

TPSTRING8_DECLARE( ss_player_quad );
TPSTRING8_DECLARE( ss_playerf_quad );
TPSTRING8_DECLARE( DialogInputMap );

// Text shadow of every text of the UI
// NOTE: inlined in the original
static void SetupTextShadow( AGUI2TextBox& a_rTextBox )
{
	a_rTextBox.SetShadow( TTRUE, 0xa0000000 );
	a_rTextBox.SetShadowAlpha( 0.6f );
	a_rTextBox.SetShadowOffset( a_rTextBox.GetScale() * 5.0f, a_rTextBox.GetScale() * 5.0f );
}

// Scales a_rTransform to a_fScaleX, a_fScaleY keeping its translation
// NOTE: inlined in the original
static void SetTransformScale( T2GUITransform& a_rTransform, TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	TVector2 vecTranslation = a_rTransform.GetTranslation();
	a_rTransform.SetIdentity();
	a_rTransform.Scale( a_fScaleX, a_fScaleY );
	a_rTransform.SetTranslation( vecTranslation );
}

// $Barnyard: FUNCTION 0062cf80
ASSeqUI::ASSeqUI( TBOOL a_bSkipName, TBOOL a_bNoSound )
    : m_bActive( TFALSE )
    , m_ePage( -1 )
    , m_bNameEdited( TFALSE )
    , m_fDialogWidth( 0.0f )
    , m_fDialogHeight( 0.0f )
    , m_bDefaultNameSet( TFALSE )
    , m_bSkipName( a_bSkipName )
    , m_bNoSound( a_bNoSound )
    , m_iSkin( 0 )
    , m_fTime( 0.0f )
    , m_fVOTime( 1.0f )
    , m_bUnk2( TFALSE )
    , m_bSkinPictureHovered( TFALSE )
    , m_bLeftArrowHovered( TFALSE )
    , m_bRightArrowHovered( TFALSE )
{
	ALocaleManager* pLocale = ALocaleManager::Interface();

	// A-Z, delete and done
	for ( TINT i = 0; i < NUM_LETTERS; i++ )
		ms_awszLetters[ i ] = pLocale->GetString( 205 + i );

	ms_awszSkinNames[ 0 ] = pLocale->GetString( 1057 );
	ms_awszSkinNames[ 1 ] = pLocale->GetString( 1058 );
	ms_awszSkinNames[ 2 ] = pLocale->GetString( 1059 );
	ms_awszSkinNames[ 5 ] = pLocale->GetString( 1060 );
	ms_awszSkinNames[ 6 ] = pLocale->GetString( 1061 );
	ms_awszSkinNames[ 4 ] = pLocale->GetString( 1062 );
	ms_awszSkinNames[ 3 ] = pLocale->GetString( 1063 );

	ms_auiUnk[ 0 ] = 161;
	ms_auiUnk[ 1 ] = 162;
	ms_auiUnk[ 2 ] = 163;
	ms_auiUnk[ 3 ] = 164;

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	m_oDialog.Create( "Border_Top_Left", "Border_Horizontal", "Border_Top_Right", "Border_Vertical", "Border_Middle", "Border_Vertical", "Border_Bottom_Left", "Border_Horizontal", "Border_Bottom_Right" );
	m_fDialogWidth  = 526.0f;
	m_fDialogHeight = 256.0f;

	// Gender buttons: male and female
	for ( TINT i = 0; i < NUM_GENDERS; i++ )
	{
		AGUI2Button& rButton = m_aGenderButtons[ i ];
		rButton.Create( pFont, 100.0f, pLocale->GetString( 1055 + i ), 2.5f );
		rButton.SetFocusedColour( 0xffffffff );
		rButton.SetEnabledColour( 0xffb56f22 );
		rButton.SetDisabledColour( 0xff4b4b4b );
		rButton.SetFocusedScale( 1.0f );
		rButton.OnFocusLost();
		rButton.SetId( i );
		SetupTextShadow( rButton.GetTextBox() );
	}

	// Skin buttons
	for ( TINT i = 0; i < NUM_SKINS; i++ )
	{
		AGUI2Button& rButton = m_aSkinButtons[ i ];
		rButton.Create( pFont, 200.0f, ms_awszSkinNames[ i ], 2.5f );
		rButton.SetFocusedColour( 0xffffffff );
		rButton.SetEnabledColour( 0xffb56f22 );
		rButton.SetDisabledColour( 0xff4b4b4b );
		rButton.SetFocusedScale( 1.0f );
		rButton.OnFocusLost();

		// NOTE: the original gets the dimensions of the button here and doesn't use them

		rButton.SetId( i );
		SetupTextShadow( rButton.GetTextBox() );
	}

	m_oNameGrid.CreateGrid( 4, 7, 0.0f, 0.0f );
	T2GUIFont* pSmallFont = T2GUIFontManager::FindFont( "rekord18" );

	m_oSkinName.Create( pFont, 0.0f );
	SetupTextShadow( m_oSkinName );

	m_aSkinArrows[ 0 ].SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "left_arrow" ) );
	m_aSkinArrows[ 1 ].SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "left_arrow2" ) );

	// The letters of the name (delete and done use the small font)
	for ( TINT i = 0; i < NUM_LETTERS; i++ )
	{
		AGUI2MenuGridTextItem& rLetter = m_aLetters[ i ];
		rLetter.Create( ( i < 26 ) ? pFont : pSmallFont, 50.0f, ms_awszLetters[ i ], 2.5f );
		rLetter.SetFocusedColour( 0xffffffff );
		rLetter.SetEnabledColour( 0xffb56f22 );
		rLetter.SetDisabledColour( 0xff4b4b4b );
		rLetter.SetFocusedScale( 1.0f );
		rLetter.OnFocusLost();

		// NOTE: the original gets the dimensions of the item here and doesn't use them

		rLetter.SetId( i );
		SetupTextShadow( rLetter.GetTextBox() );
	}

	m_oNameGrid.SetItemActivationCallback( OnLetterActivatedCallback );
	m_oNameGrid.SetCallbackUserData( this );
	m_oNameGrid.SetFocusAt( &m_aLetters[ NUM_LETTERS - 1 ] );
	m_oNameGrid.SetFocusSound( 166 );
	m_oNameGrid.SetBackSound( 167 );
	m_oNameGrid.SetSelectSound( 168 );

	// The letters are in 4 rows of 7, moving past an edge wraps around
	// NOTE: unrolled in the original
	for ( TINT i = 0; i < NUM_LETTERS; i++ )
	{
		const TINT iRow    = i / 7;
		const TINT iColumn = i % 7;

		m_aLetters[ i ].SetSideElements(
		    &m_aLetters[ iRow * 7 + ( iColumn + 6 ) % 7 ],
		    &m_aLetters[ iRow * 7 + ( iColumn + 1 ) % 7 ],
		    &m_aLetters[ ( ( iRow + 3 ) % 4 ) * 7 + iColumn ],
		    &m_aLetters[ ( ( iRow + 1 ) % 4 ) * 7 + iColumn ]
		);
	}

	pFont = T2GUIFontManager::FindFont( "rekord26" );

	// Title of the page
	m_oTitle.Create( pFont, 1.0f );
	m_oTitle.SetTextAndFitWidth( pLocale->GetString( 323 ) );
	m_oTitle.SetColour( 0xfffff0af );
	m_oTitle.SetTransform( 0.0f, 2.0f );

	T2GUITransform oTitleScale;
	oTitleScale.m_aMatrixRows[ 0 ] = TVector2( 1.1f, 0.0f );
	oTitleScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 1.1f );
	oTitleScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oTitle.GetTransform().PreMultiply( oTitleScale );

	SetupTextShadow( m_oTitle );

	T2GUITextureSection* pPanel = T2GUITextureSectionManager::GetTextureSection( "Panel_01" );
	m_oTitleBackground.SetTextureSection( pPanel );
	m_oTitleBackground.SetDimensions( TMath::Max( m_oTitle.GetWidth() * 1.05f, pPanel->GetWidth() ), pPanel->GetHeight() );
	m_oTitleBackground.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitleBackground.SetTransform( -7.0f, 0.0f );

	// The name
	m_oName.Create( pFont, 1.0f );
	m_oName.SetTextAndFitWidth( m_strName.GetString() );
	SetupTextShadow( m_oName );
	m_oName.SetTransform( 0.0f, 3.0f );

	m_oNameBackground.SetTextureSection( pPanel );
	m_oNameBackground.SetDimensions( pFont->GetTextWidth( L"W" ) * 12.0f, pPanel->GetHeight() );
	m_oNameBackground.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );

	T2GUITransform oNameScale;
	oNameScale.m_aMatrixRows[ 0 ] = TVector2( 1.4f, 0.0f );
	oNameScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 1.4f );
	oNameScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oNameBackground.GetTransform().PreMultiply( oNameScale );

	// Gender page
	m_oGenderMenu.SetDimensions( 200.0f, 200.0f );
	m_oGenderMenu.SetItemSpacing( 10.0f );
	m_oGenderMenu.SetFocusAt( m_aGenderButtons[ 0 ] );
	m_oGenderMenu.SetFocus( TTRUE );
	m_oGenderMenu.Show();
	m_oGenderMenu.SetItemActivationCallback( OnMenuItemActivatedCallback );
	m_oGenderMenu.SetItemFocusCallback( OnMenuItemFocusedCallback );
	m_oGenderMenu.SetCallbackUserData( this );
	m_oGenderMenu.SetFocusSound( 166 );
	m_oGenderMenu.SetBackSound( 167 );
	m_oGenderMenu.SetSelectSound( 168 );
	m_oGenderMenu.SetNegativeSound( 163 );
	m_oGenderMenu.SetTransform( 0.0f, m_oTitleBackground.GetHeight() * 0.5f - 20.0f );

	// Skin page
	m_oSkinMenu.SetDimensions( 200.0f, 200.0f );
	m_oSkinMenu.SetFocusAt( m_aSkinButtons[ 0 ] );
	m_oSkinMenu.SetFocus( TTRUE );
	m_oSkinMenu.Show();
	m_oSkinMenu.SetItemActivationCallback( OnMenuItemActivatedCallback );
	m_oSkinMenu.SetCallbackUserData( this );
	m_oSkinMenu.SetFocusSound( 166 );
	m_oSkinMenu.SetBackSound( 167 );
	m_oSkinMenu.SetSelectSound( 168 );
	m_oSkinMenu.SetNegativeSound( 163 );
	m_oSkinMenu.SetTransform( 0.0f, m_oTitleBackground.GetHeight() * 0.5f + 100.0f );
}

// $Barnyard: FUNCTION 0062b8a0
// $Barnyard: FUNCTION 0062b6e0
ASSeqUI::~ASSeqUI()
{
}

// $Barnyard: FUNCTION 0062b850
void ASSeqUI::CycleSkin( TINT a_iDirection )
{
	m_iSkin += a_iDirection;

	while ( TTRUE )
	{
		if ( m_iSkin < 0 )
			m_iSkin = NUM_SKINS - 1;
		else if ( m_iSkin >= NUM_SKINS )
			m_iSkin = 0;

		// Skins 3 and 4 can't be chosen
		if ( m_iSkin != 3 && m_iSkin != 4 )
			break;

		m_iSkin += a_iDirection;
	}
}

// $Barnyard: FUNCTION 0062b8c0
void ASSeqUI::Reset()
{
	if ( !m_bNoSound )
	{
		AVOStream* pVOStream = AVOStream::GetSingleton();

		if ( pVOStream->IsPlaying() )
			pVOStream->Stop();
	}

	for ( TINT i = 0; i < NUM_GENDERS; i++ )
	{
		m_aGenderButtons[ i ].RemoveSelf();
	}

	m_oGenderMenu.RemoveSelf();

	for ( TINT i = 0; i < NUM_SKINS; i++ )
	{
		m_aSkinButtons[ i ].RemoveSelf();
	}

	m_oSkinMenu.RemoveSelf();

	for ( TINT i = 0; i < NUM_LETTERS; i++ )
	{
		m_aLetters[ i ].RemoveSelf();
	}

	m_oNameGrid.RemoveSelf();
	m_oName.RemoveSelf();
	m_oNameBackground.RemoveSelf();
	m_oTitle.RemoveSelf();
	m_oTitleBackground.RemoveSelf();
	m_oDialog.RemoveSelf();

	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );

	m_bUnk2               = TFALSE;
	m_bSkinPictureHovered = TFALSE;
	m_bLeftArrowHovered   = TFALSE;
	m_bRightArrowHovered  = TFALSE;

	for ( TINT i = 0; i < 2; i++ )
	{
		m_aSkinArrows[ i ].RemoveSelf();
	}

	m_oSkinPicture.RemoveSelf();
	m_oSkinName.RemoveSelf();
}

// $Barnyard: FUNCTION 0062ba30
void ASSeqUI::Update( TFLOAT a_fDeltaTime )
{
	// The focused item pulses
	const TFLOAT fPulse = TMath::Sin( TMath::TWO_PI * g_oSystemManager.GetScheduler()->GetTotalTime() ) * 0.05f + 1.0f;

	if ( m_ePage == PAGE_GENDER )
	{
		SetTransformScale( m_oGenderMenu.GetFocusedMenuItem()->GetTransform(), fPulse, fPulse );
		m_oGenderMenu.Update( a_fDeltaTime );
	}
	else if ( m_ePage == PAGE_SKIN )
	{
		// What the mouse cursor is over
		T2GUITransform oInvTransform;
		const TVector2 vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();
		TVector2       vecLocalPos;

		m_oSkinPicture.GetInvScreenTransform( oInvTransform );
		oInvTransform.Transform( vecLocalPos, vecCursorPos );
		m_bSkinPictureHovered = m_oSkinPicture.IsPointInside( vecLocalPos.x, vecLocalPos.y );

		if ( !m_bSkinPictureHovered )
		{
			m_aSkinArrows[ 0 ].GetInvScreenTransform( oInvTransform );
			oInvTransform.Transform( vecLocalPos, vecCursorPos );
			m_bLeftArrowHovered = m_aSkinArrows[ 0 ].IsPointInside( vecLocalPos.x, vecLocalPos.y );

			if ( !m_bSkinPictureHovered && !m_bLeftArrowHovered )
			{
				m_aSkinArrows[ 1 ].GetInvScreenTransform( oInvTransform );
				oInvTransform.Transform( vecLocalPos, vecCursorPos );
				m_bRightArrowHovered = m_aSkinArrows[ 1 ].IsPointInside( vecLocalPos.x, vecLocalPos.y );
			}
		}

		m_bUnk2 = TFALSE;

		// The hovered arrow grows
		const TFLOAT fLeftScale  = ( m_bLeftArrowHovered == TTRUE ) ? 1.25f : 1.0f;
		const TFLOAT fRightScale = ( m_bRightArrowHovered == TTRUE ) ? 1.25f : 1.0f;
		const TFLOAT fSpeed      = a_fDeltaTime * 10.0f;

		for ( TINT i = 0; i < 2; i++ )
		{
			T2GUITransform& rTransform = m_aSkinArrows[ i ].GetTransform();
			const TFLOAT    fScale     = ( i == 0 ) ? fLeftScale : fRightScale;

			const TFLOAT fScaleX = TMath::LERPClamped( rTransform.m_aMatrixRows[ 0 ].x, fScale, fSpeed );
			const TFLOAT fScaleY = TMath::LERPClamped( rTransform.m_aMatrixRows[ 1 ].y, fScale, fSpeed );

			SetTransformScale( rTransform, fScaleX, fScaleY );
		}

		m_oSkinMenu.Update( a_fDeltaTime );
	}
	else if ( m_ePage == PAGE_NAME )
	{
		if ( m_bNoSound == TFALSE )
		{
			// The voice-over asking for the name, once the previous line is over
			AVOStream* pVOStream = AVOStream::GetSingleton();

			if ( m_fVOTime > 0.0f && !pVOStream->IsPlaying() )
			{
				if ( m_fVOTime - a_fDeltaTime < 0.0f )
					pVOStream->Play( 49, TFALSE );

				m_fVOTime -= a_fDeltaTime;
			}
		}

		// Done can only be chosen once there's a name
		AGUI2MenuGridTextItem& rDone = m_aLetters[ NUM_LETTERS - 1 ];

		if ( m_strName.Length() >= 1 )
		{
			if ( rDone.IsEnabled() == TFALSE )
				m_oNameGrid.SetItemEnabled( rDone, TTRUE );
		}
		else if ( rDone.IsEnabled() == TTRUE )
		{
			m_oNameGrid.SetItemEnabled( rDone, TFALSE );
		}

		SetTransformScale( m_oNameGrid.GetFocusedMenuItem()->GetTransform(), fPulse, fPulse );
		m_oNameGrid.Update( a_fDeltaTime );
	}

	m_fTime += a_fDeltaTime;
}

// NOTE: inlined in the original
void ASSeqUI::UpdateNameText()
{
	m_oName.SetTextAndFitWidth( m_strName.GetString() );
}

// $Barnyard: FUNCTION 0062be70
void ASSeqUI::OnLetterActivated( AGUI2MenuGridItem* a_pItem )
{
	const TINT    iLetter          = TSTATICCAST( AGUI2MenuGridTextItem, a_pItem )->GetId();
	const TWCHAR* wszDefaultMale   = ALocaleManager::Interface()->GetString( 1079 );
	const TWCHAR* wszDefaultFemale = ALocaleManager::Interface()->GetString( 1080 );

	if ( iLetter == NUM_LETTERS - 2 )
	{
		// Delete

		// The default name goes away once the name is edited
		if ( m_bNameEdited == TFALSE && ( m_strName.Compare( wszDefaultMale ) == 0 || m_strName.Compare( wszDefaultFemale ) == 0 ) )
		{
			// NOTE: the original removes every character instead
			m_strName.Truncate( 0 );
			m_bNameEdited = TTRUE;
		}

		if ( m_strName.Length() > 0 )
		{
			// NOTE: the original removes the last character instead
			m_strName.Truncate( m_strName.Length() - 1 );
			UpdateNameText();
		}
	}
	else if ( iLetter == NUM_LETTERS - 1 )
	{
		// Done
		if ( m_strName.Length() == 0 )
			m_strName.Copy( ( APlayerProfileManager::GetSingleton()->GetMainGender() != APlayerProfile::GENDER_MALE ) ? wszDefaultFemale : wszDefaultMale );

		// First letter in upper case, the rest in lower case
		// NOTE: the original removes the first character and inserts it back in upper case
		m_strName.MakeLower();
		m_strName[ 0 ] = T2String16::CharToUpperCase( TUINT8( m_strName[ 0 ] ) );

		APlayerProfileManager::GetSingleton()->GetMainProfile()->SetName( m_strName );

		Reset();
		m_bActive = TFALSE;

		if ( m_bNoSound == TFALSE )
			AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetForceHide( TTRUE );
	}
	else
	{
		// The default name goes away once the name is edited
		if ( m_bNameEdited == TFALSE && ( m_strName.Compare( wszDefaultMale ) == 0 || m_strName.Compare( wszDefaultFemale ) == 0 ) )
		{
			m_strName.Truncate( 0 );
			m_bNameEdited = TTRUE;
		}

		// At most 10 letters
		if ( m_strName.Length() < 10 )
		{
			m_strName.Concat( ms_awszLetters[ iLetter ] );
			UpdateNameText();
		}
	}
}

// $Barnyard: FUNCTION 0062c0a0
void ASSeqUI::SetPage( PAGE a_ePage )
{
	if ( m_ePage == a_ePage )
		return;

	m_ePage = a_ePage;

	if ( a_ePage == PAGE_GENDER )
	{
		m_oTitle.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 1052 ) );
		m_oTitleBackground.SetWidth( m_oTitle.GetWidth() * 1.2f );

		m_oGenderMenu.Show();
		m_oDialog.SetDialogSize( TMath::Max( m_oGenderMenu.GetWidth(), m_fDialogWidth * 0.9f ), m_oTitleBackground.GetHeight() * 0.5f + m_fDialogHeight * 0.9f );

		m_oSkinMenu.Hide();
		m_oNameGrid.Hide();
		m_oNameBackground.Hide();
		m_oSkinPicture.Hide();
	}
	else if ( a_ePage == PAGE_SKIN )
	{
		m_oTitle.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 1053 ) );
		m_oTitleBackground.SetWidth( m_oTitle.GetWidth() * 1.2f );

		const TCHAR* szPicture = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( m_iSkin, APlayerProfile::GENDER_NONE );
		m_oSkinPicture.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( szPicture ) );

		m_oDialog.SetDialogSize( TMath::Max( m_oSkinMenu.GetWidth(), m_fDialogWidth * 0.9f ), m_oTitleBackground.GetHeight() * 0.5f + m_fDialogHeight * 0.9f );
		m_oSkinPicture.SetTransform( 0.0f, m_oDialog.GetHeight() * -0.1f );

		m_oGenderMenu.Hide();
		m_oNameGrid.Hide();
		m_oNameBackground.Hide();
		m_oSkinPicture.Show();

		if ( m_bNoSound == TFALSE )
			AVOStream::GetSingleton()->Play( 47, TFALSE );
	}
	else if ( a_ePage == PAGE_NAME )
	{
		if ( m_bSkipName == TTRUE )
		{
			// Done without a name
			APlayerProfileManager::GetSingleton()->GetMainProfile()->SetName( m_strName );

			Reset();
			m_bActive = TFALSE;

			if ( m_bNoSound == TFALSE )
				AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetForceHide( TTRUE );

			return;
		}

		m_oTitle.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 1054 ) );
		m_oTitleBackground.SetWidth( m_oTitle.GetWidth() * 1.2f );

		m_oNameGrid.Show();
		m_oDialog.SetDialogSize( TMath::Max( m_oNameGrid.GetWidth(), m_fDialogWidth * 0.9f ), m_oTitleBackground.GetHeight() * 0.5f + m_fDialogHeight * 0.9f );
		m_oNameBackground.Show();

		m_oSkinMenu.Hide();
		m_oGenderMenu.Hide();
		m_oSkinPicture.Hide();

		if ( m_bNoSound == TTRUE )
			return;

		AVOStream* pVOStream = AVOStream::GetSingleton();

		if ( pVOStream->IsPlaying() )
			pVOStream->Stop();

		pVOStream->Play( 48, TFALSE );
	}

	if ( m_bNoSound == TFALSE )
	{
		AMenuNavHudElement* pMenuNav = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();

		if ( m_ePage != PAGE_GENDER )
			pMenuNav->SetDefaultTexts();
		else
			pMenuNav->SetTexts( 1047, -1, -1 );
	}
}

// $Barnyard: FUNCTION 0062c4d0
TBOOL ASSeqUI::Show( const TCHAR* a_szUnused )
{
	for ( TINT i = 0; i < NUM_SKINS; i++ )
		m_oSkinMenu.AddMenuItem( m_aSkinButtons[ i ] );

	m_oSkinMenu.ReflowChildrenVertically();

	for ( TINT i = 0; i < NUM_LETTERS; i++ )
		m_oNameGrid.AddMenuGridItem( m_aLetters[ i ] );

	m_oNameGrid.ReflowChildren();

	m_oGenderMenu.AddMenuItem( m_aGenderButtons[ 0 ] );
	m_oGenderMenu.AddMenuItem( m_aGenderButtons[ 1 ] );
	m_oGenderMenu.ReflowChildrenVertically();

	m_bDefaultNameSet = TFALSE;
	m_bNameEdited     = TFALSE;

	T2GUIElement* pRoot = AGUI2::GetRootElement();

	if ( m_bNoSound == TFALSE )
	{
		// On the left of the screen
		m_oDialog.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		m_oDialog.SetTransform( 10.0f, 0.0f );
	}

	pRoot->AddChildTail( m_oDialog );
	m_oDialog.AddChildTail( m_oGenderMenu );
	m_oGenderMenu.Show();

	m_oTitle.Show();
	m_oTitleBackground.AddChildTail( m_oTitle );
	m_oDialog.AddChildTail( m_oTitleBackground );
	m_oTitleBackground.Show();

	m_oNameBackground.AddChildTail( m_oName );
	m_oDialog.AddChildTail( m_oNameBackground );
	m_oNameBackground.Hide();

	m_oDialog.AddChildTail( m_oSkinMenu );
	m_oSkinMenu.Hide();

	m_oDialog.AddChildTail( m_oNameGrid );
	m_oNameGrid.Hide();

	m_bActive = TTRUE;

	AGameStateController::GetSingleton()->GetCurrentState()->SetInputMap( TPS8( DialogInputMap ) );
	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );

	// The skin
	m_iSkin = 0;
	m_oSkinPicture.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( 0, APlayerProfile::GENDER_MALE ) ) );

	m_oSkinPicture.AddChildTail( m_oSkinName );
	m_oSkinName.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oSkinName.SetTransform( 0.0f, 20.0f );

	m_aSkinArrows[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_aSkinArrows[ 0 ].SetTransform( -10.0f, 0.0f );
	m_aSkinArrows[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aSkinArrows[ 1 ].SetTransform( 10.0f, 0.0f );

	m_oSkinPicture.AddChildTail( m_aSkinArrows[ 0 ] );
	m_oSkinPicture.AddChildTail( m_aSkinArrows[ 1 ] );

	m_oSkinName.SetTextAndFitWidth( ms_awszSkinNames[ m_iSkin ] );
	m_oSkinPicture.Hide();
	m_oDialog.AddChildTail( m_oSkinPicture );

	SetPage( PAGE_GENDER );

	AGameStateController::GetSingleton()->GetCurrentState()->GetHUDParams().SetFlags( AHudElement::MASK_LETTERBOX | AHudElement::MASK_MENU_NAV );

	if ( m_bNoSound == TFALSE )
		AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetForceHide( TFALSE );

	return TTRUE;
}

// $Barnyard: FUNCTION 0062c790
void ASSeqUI::SetGender( TINT a_iGender )
{
	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();

	if ( pProfileManager->GetMainGender() == a_iGender )
		return;

	pProfileManager->SetMainGender( a_iGender );

	if ( m_bNoSound == TTRUE )
		return;

	// Swap the model of the player in the sequence (actor 18)
	TPString8   strModel = ( a_iGender == APlayerProfile::GENDER_MALE ) ? TPS8( ss_player_quad ) : TPS8( ss_playerf_quad );
	AUnitActor* pActor   = TSTATICCAST( AUnitActor, AUnitManager::GetSingleton()->FindActorBySSeqId( 18 ) );

	TMatrix44 mTransform;
	pActor->GetAnimatableObject()->GetModelInstance()->GetLocalTransform( mTransform );

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObject*        pObject         = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( strModel ), pActor, 0 );
	pObject->GetModelInstance()->SetTransform( mTransform );

	AToshiAnimationInterface& rAnimInterface = pObject->GetToshiAnimInterface();
	rAnimInterface.SetFlags( rAnimInterface.GetFlags() | AToshiAnimationInterface::FLAGS_ALWAYS_UPDATE );

	pActor->ReplaceAnimatableObject( pObject );

	// Idle animation of the UI
	AModelInstanceRef pModelInstance = pObject->GetModelInstance();
	TINT              iSequence      = pModelInstance->GetSkeleton()->GetSequenceID( "ss01_gui_idle", 0 );
	pActor->SetAnimation( pModelInstance->GetSkeletonInstance()->AddAnimation( TUINT16( iSequence ), 1.0f, 0.0f ) );

	pObject->ApplyMaterial( m_iSkin, TTRUE );
	pObject->SetSkeletonUpdating( TTRUE, TTRUE );
}

// $Barnyard: FUNCTION 0062c9d0
void ASSeqUI::OnLetterActivatedCallback( void* a_pUserData, AGUI2MenuGridItem* a_pItem )
{
	TSTATICCAST( ASSeqUI, a_pUserData )->OnLetterActivated( a_pItem );
}

// $Barnyard: FUNCTION 0062c9f0
void ASSeqUI::OnMenuItemActivated( AGUI2MenuItem* a_pItem )
{
	const TINT iId = TSTATICCAST( AGUI2Button, a_pItem )->GetId();

	if ( m_ePage == PAGE_GENDER )
	{
		SetGender( iId );

		if ( m_bDefaultNameSet == TFALSE )
		{
			// The default name of the gender
			m_strName.Copy( ALocaleManager::Interface()->GetString( ( iId != APlayerProfile::GENDER_MALE ) ? 1080 : 1079 ) );
			UpdateNameText();
			m_bDefaultNameSet = TTRUE;
		}

		SetPage( PAGE_SKIN );
	}
	else if ( m_ePage == PAGE_SKIN )
	{
		if ( m_bNoSound == TFALSE )
			TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->FindActorBySSeqId( 18 ) )->GetAnimatableObject()->ApplyMaterial( iId, TTRUE );

		APlayerProfileManager::GetSingleton()->GetMainProfile()->SetCostume( iId );
		SetPage( PAGE_NAME );
	}
}

// $Barnyard: FUNCTION 0062cae0
void ASSeqUI::OnMenuItemFocusedCallback( void* a_pUserData, AGUI2MenuItem* a_pOldFocus, AGUI2MenuItem* a_pNewFocus )
{
	TSTATICCAST( ASSeqUI, a_pUserData )->SetGender( TSTATICCAST( AGUI2Button, a_pNewFocus )->GetId() );
}

// $Barnyard: FUNCTION 0062cf00
TBOOL ASSeqUI::ProcessInputEvent( const TInputInterface::InputEvent* a_pEvent )
{
	switch ( m_ePage )
	{
		case PAGE_GENDER:
			return m_oGenderMenu.ProcessInputEvent( a_pEvent );
		case PAGE_SKIN:
			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN && a_pEvent->GetDoodad() == TInputDeviceMouse::BUTTON_1 )
			{
				if ( m_bSkinPictureHovered == TTRUE )
					return ProcessInputCommand( AInputCommand_OK, a_pEvent );

				if ( m_bLeftArrowHovered == TTRUE )
					return ProcessInputCommand( AInputCommand_Left, a_pEvent );

				if ( m_bRightArrowHovered == TTRUE )
					return ProcessInputCommand( AInputCommand_Right, a_pEvent );

				return TTRUE;
			}

			return TFALSE;
		case PAGE_NAME:
			return m_oNameGrid.ProcessInputEvent( a_pEvent );
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0062cb00
TBOOL ASSeqUI::ProcessInputCommand( AInputCommand a_eCommand, const TInputInterface::InputEvent* a_pEvent )
{
	if ( m_bActive != TTRUE )
		return TFALSE;

	const TInputInterface::EVENT_TYPE eEventType = a_pEvent->GetEventType();

	if ( eEventType != TInputInterface::EVENT_TYPE_GONE_DOWN && eEventType != TInputInterface::EVENT_TYPE_REPEAT && eEventType != TInputInterface::EVENT_TYPE_VIRTUAL )
		return TFALSE;

	if ( a_eCommand == AInputCommand_Cancel )
	{
		ASoundManager::GetSingleton()->PlayCue( ( m_ePage != PAGE_GENDER || m_bNoSound == TTRUE ) ? 167 : 163 );

		if ( m_ePage == PAGE_GENDER )
		{
			if ( m_bNoSound != TTRUE )
				return TFALSE;

			Reset();
			m_bActive = TFALSE;
			return TTRUE;
		}
		else if ( m_ePage == PAGE_SKIN )
		{
			SetPage( PAGE_GENDER );
			m_bDefaultNameSet = TFALSE;
			return TTRUE;
		}
		else if ( m_ePage == PAGE_NAME )
		{
			SetPage( PAGE_SKIN );
			return TTRUE;
		}

		return TFALSE;
	}

	if ( m_ePage == PAGE_GENDER )
	{
		if ( a_eCommand != AInputCommand_Start )
			return m_oGenderMenu.ProcessInputCommand( a_eCommand, a_pEvent );

		OnMenuItemActivated( m_oGenderMenu.GetFocusedMenuItem() );
		return TTRUE;
	}

	if ( m_ePage == PAGE_SKIN )
	{
		if ( a_eCommand == AInputCommand_OK )
		{
			// Clicking an arrow is the same as pressing left or right
			if ( m_bLeftArrowHovered == TTRUE )
				a_eCommand = AInputCommand_Left;
			else if ( m_bRightArrowHovered == TTRUE )
				a_eCommand = AInputCommand_Right;
			else
				a_eCommand = AInputCommand_Start;
		}

		if ( a_eCommand == AInputCommand_Left || a_eCommand == AInputCommand_Right )
		{
			ASoundManager::GetSingleton()->PlayCue( 166 );

			// The arrow pops
			const TINT iArrow = ( a_eCommand == AInputCommand_Left ) ? 0 : 1;
			SetTransformScale( m_aSkinArrows[ iArrow ].GetTransform(), 1.25f, 1.25f );

			CycleSkin( ( a_eCommand == AInputCommand_Left ) ? -1 : 1 );

			m_oSkinName.SetTextAndFitWidth( ms_awszSkinNames[ m_iSkin ] );
			m_oSkinPicture.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( m_iSkin, APlayerProfile::GENDER_NONE ) ) );

			if ( m_bNoSound == TFALSE )
				TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->FindActorBySSeqId( 18 ) )->GetAnimatableObject()->ApplyMaterial( m_iSkin, TTRUE );

			a_pEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		}

		if ( a_eCommand == AInputCommand_Start )
		{
			ASoundManager::GetSingleton()->PlayCue( 168 );
			APlayerProfileManager::GetSingleton()->GetMainProfile()->SetCostume( m_iSkin );
			SetPage( PAGE_NAME );
			return TTRUE;
		}

		return TFALSE;
	}

	if ( m_ePage == PAGE_NAME )
	{
		if ( a_eCommand != AInputCommand_Start )
			return m_oNameGrid.ProcessInputCommand( a_eCommand, a_pEvent );

		// Start is the same as done
		OnLetterActivated( &m_aLetters[ NUM_LETTERS - 1 ] );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0062cee0
void ASSeqUI::OnMenuItemActivatedCallback( void* a_pUserData, AGUI2MenuItem* a_pItem )
{
	TSTATICCAST( ASSeqUI, a_pUserData )->OnMenuItemActivated( a_pItem );
}
