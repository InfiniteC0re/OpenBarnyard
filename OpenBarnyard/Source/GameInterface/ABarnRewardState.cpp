#include "pch.h"
#include "ABarnRewardState.h"
#include "AGameStateController.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUISystem.h"
#include "Helpers/AGameLoader.h"
#include "ARandom.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Units/AUnitType.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitFactory.h"
#include "Units/AObjectTraits.h"
#include "Units/AUnitModelStaticData.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004311d0
TDEFINE_CLASS_NORUNTIME( ABarnRewardState );

TPSTRING8_DECLARE( buck_1 );
TPSTRING8_DECLARE( bkg_End_wood );
TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 00431220
ABarnRewardState::ABarnRewardState( TYPE a_eType, TINT a_iMoney )
{
	m_iMoney          = a_iMoney;
	m_eType           = a_eType;
	m_pObject         = TNULL;
	m_pUnitType       = TNULL;
	m_oModelTransform = TMatrix44::IDENTITY;
	m_fScreenSize     = 0.0f;
	m_fRotation       = 0.0f;
	m_ePage           = PAGE_REWARD;
	m_iPicture        = -1;
	m_pUnitType       = AUnitManager::GetSingleton()->GetUnitFactory()->Find( TPS8( buck_1 ) );
}

// $Barnyard: FUNCTION 00431360
ABarnRewardState::ABarnRewardState( TYPE a_eType, const TPString8& a_rcItemType, TINT a_iPicture )
{
	m_eType           = a_eType;
	m_pObject         = TNULL;
	m_pUnitType       = TNULL;
	m_iMoney          = -1;
	m_oModelTransform = TMatrix44::IDENTITY;
	m_fScreenSize     = 0.0f;
	m_fRotation       = 0.0f;
	m_ePage           = PAGE_REWARD;
	m_iPicture        = a_iPicture;
	m_pUnitType       = AUnitManager::GetSingleton()->GetUnitFactory()->Find( a_rcItemType );
}

// $Barnyard: FUNCTION 004314b0
// $Barnyard: FUNCTION 00432360
ABarnRewardState::~ABarnRewardState()
{
	AGUISystem::GetSingleton()->SetPicture( TPString8() );
	AGUISystem::GetSingleton()->ResetPicture();

	for ( TINT i = 0; i < TARRAYSIZE( m_aFlares ); i++ )
		m_aFlares[ i ].oRectangle.RemoveSelf();

	m_oFrame.RemoveSelf();
	m_oPicturePage.RemoveSelf();
	m_oTitle.RemoveSelf();
	m_oSubtitle.RemoveSelf();
	m_oName.RemoveSelf();
	m_oPressOK.RemoveSelf();
	m_oModelRenderer.RemoveSelf();

	if ( m_pObject )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pObject );
		m_pObject = TNULL;
	}
}

// $Barnyard: FUNCTION 004321b0
TBOOL ABarnRewardState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand != AInputCommand_OK || a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );

	if ( m_ePage == PAGE_REWARD )
	{
		if ( m_eType != TYPE_MONEY && m_iPicture != -1 && !AGameLoader::IsSimLoadingPictureUnlocked( m_iPicture ) )
		{
			// Show the picture the reward unlocks
			m_ePage = PAGE_PICTURE;
			m_oFrame.Hide();
			m_oPressOK.RemoveSelf();

			AGameLoader::UnlockSimLoadingPicture( m_iPicture );

			TPString8 strPicture;
			AGUISystem::GetSingleton()->SetPicture( AGameLoader::GetSimLoadingPicture( strPicture, m_iPicture ) );

			m_oPicturePage.AddChildTail( m_oPressOK );
			m_oPicturePage.Show();
			return TTRUE;
		}
	}
	else if ( m_ePage != PAGE_PICTURE )
	{
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}

	// Close the screen
	AGameStateController* pController = AGameStateController::GetSingleton();
	pController->AddFlags( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_REMOVE_STATE );
	pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );

	return TTRUE;
}

// $Barnyard: FUNCTION 00431f90
TBOOL ABarnRewardState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_ePage != PAGE_REWARD )
		return TTRUE;

	// NOTE: retail only animates the first 3 of the 4 flares
	for ( TINT i = 0; i < 3; i++ )
	{
		Flare& rFlare = m_aFlares[ i ];

		TFLOAT fSize = ( TMath::Sin( rFlare.fFrequency * rFlare.fTime * TMath::TWO_PI ) * 0.25f + 0.75f ) * m_fScreenSize;
		rFlare.oRectangle.SetDimensions( fSize, fSize );
		rFlare.fTime += a_fDeltaTime;
	}

	if ( m_pObject )
	{
		// Spin the model around
		AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, m_pUnitType->FindStaticData( &TGetClass( AObjectTraits ) ) );

		TMatrix44 matTransform = TMatrix44::IDENTITY;
		matTransform.RotateX( TMath::HALF_PI );
		matTransform.RotateZ( m_fRotation );
		matTransform.Rotate( pTraits->m_vecHoldRotate.x, pTraits->m_vecHoldRotate.y, pTraits->m_vecHoldRotate.z );

		TFLOAT fScale = pTraits->m_fItemScale;
		matTransform.Scale( fScale, fScale, fScale );

		TMatrix44::TransformVector( matTransform.GetTranslation(), matTransform, pTraits->m_vecItemOffset );

		// Center the model around its bounding sphere
		TVector4 vecCenter = m_pObject->GetModelInstance()->GetSceneObject()->GetInstance()->GetModel()->GetLOD( 0 ).BoundingSphere.AsVector4();
		vecCenter.x        = -vecCenter.x;
		vecCenter.y        = -vecCenter.y;
		vecCenter.z        = -vecCenter.z;
		TMatrix44::TransformVector( matTransform.GetTranslation3(), matTransform, vecCenter.AsVector3() );

		m_pObject->GetModelInstance()->SetTransform( matTransform );

		m_fRotation += a_fDeltaTime;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00431680
void ABarnRewardState::OnInsertion()
{
	AGameState::OnInsertion();

	AGameStateController::GetSingleton()->AddFlags( AGameStateController::FLAGS_FADING_IN );
	SetInputMap( TPS8( DialogInputMap ) );

	T2GUIFont*    pFont        = T2GUIFontManager::FindFont( "Rekord26" );
	T2GUIElement* pRootElement = AGUI2::GetRootElement();

	m_fScreenSize      = TMath::Max( pRootElement->GetWidth(), pRootElement->GetHeight() );
	TFLOAT fScreenSize = TMath::Max( pRootElement->GetWidth(), pRootElement->GetHeight() );

	// The OK prompt
	TWCHAR wszPrompt[ 64 ];
	TStringManager::String16Format( wszPrompt, TARRAYSIZE( wszPrompt ), L"%s %s", L"%c_DialogInputMap[OK]", ALocaleManager::Interface()->GetString( 279 ) );
	ALocale::FormatInputTags( m_wszPressOK, TARRAYSIZE( m_wszPressOK ), wszPrompt );

	m_oPressOK.Create( pFont, pFont->GetTextWidth( m_wszPressOK, 1.0f ) );
	m_oPressOK.SetText( m_wszPressOK );
	m_oPressOK.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );

	if ( m_ePage == PAGE_REWARD )
	{
		AGUISystem::GetSingleton()->SetPicture( TPS8( bkg_End_wood ) );

		m_oFrame.SetDimensions( fScreenSize * 0.9f, pRootElement->GetHeight() - fScreenSize * 0.1f );
		m_oFrame.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
		pRootElement->AddChildTail( m_oFrame );

		// The flares behind the model, each one turned a quarter more than the previous
		TRandom&             rRandom       = ARandom::GetSingleton()->m_oRandom;
		T2GUITextureSection* pFlareSection = T2GUITextureSectionManager::GetTextureSection( "Flare" );
		TFLOAT               fAngle        = 0.0f;

		for ( TINT i = 0; i < TARRAYSIZE( m_aFlares ); i++ )
		{
			Flare& rFlare = m_aFlares[ i ];
			rFlare.oRectangle.SetTextureSection( *pFlareSection );
			rFlare.oRectangle.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
			rFlare.oRectangle.SetAlpha( 0.75f );

			rFlare.fFrequency = rRandom.GetFloatMinMax( 0.25f, 0.5f );
			rFlare.fTime      = rRandom.GetFloat() * ( 1.0f / rFlare.fFrequency );
			m_oFrame.AddChildTail( rFlare.oRectangle );

			rFlare.oRectangle.SetTransform( 0.0f, 0.0f, rRandom.GetFloatMinMax( -TMath::PI / 8.0f, TMath::PI / 8.0f ) + fAngle );
			fAngle += TMath::HALF_PI * 0.5f;
		}

		m_aFlares[ 0 ].oRectangle.SetDimensions( m_fScreenSize, m_fScreenSize );

		for ( TINT i = 0; i < TARRAYSIZE( m_aFlares ); i++ )
		{
			Flare& rFlare = m_aFlares[ i ];
			TFLOAT fSize  = ( TMath::Sin( rFlare.fFrequency * rFlare.fTime * TMath::TWO_PI ) * 0.25f + 0.75f ) * m_fScreenSize;
			rFlare.oRectangle.SetDimensions( fSize, fSize );
		}

		// The model of the reward
		AUnitModelStaticData* pModelData = TSTATICCAST( AUnitModelStaticData, m_pUnitType->FindStaticData( &TGetClass( AUnitModelStaticData ) ) );

		if ( pModelData )
		{
			AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
			m_pObject                                = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( pModelData->GetModelName( 0 ) ), TNULL, 0 );

			if ( m_pObject )
			{
				AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, m_pUnitType->FindStaticData( &TGetClass( AObjectTraits ) ) );

				if ( pTraits )
				{
					TVector4 vecCenter = m_pObject->GetModelInstance()->GetSceneObject()->GetInstance()->GetModel()->GetLOD( 0 ).BoundingSphere.AsVector4();
					vecCenter.z        = -vecCenter.z;

					m_oModelTransform.GetTranslation() = TVector4(
					    pTraits->m_vecItemOffset.x - vecCenter.x,
					    pTraits->m_vecItemOffset.y - vecCenter.y,
					    vecCenter.z + pTraits->m_vecItemOffset.z,
					    1.0f
					);
				}

				TFLOAT fModelSize = fScreenSize * 0.4f;
				m_oModelRenderer.Create( fModelSize, fModelSize, TFALSE, TFALSE );
				m_oModelRenderer.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
				m_oModelRenderer.AddObject( m_pObject, TNULL, TNULL );
				m_oModelRenderer.SetCameraPosition( TVector4( 0.0f, 0.0f, -1.0f, 1.0f ) );
				m_oModelRenderer.SetLightDirection( TVector4( 0.0f, 0.0f, 1.0f, 1.0f ) ); // 0077d004
				m_oModelRenderer.GetCamera()->SetFOV( TMath::PI / 3.0f );
				m_oModelRenderer.GetCamera()->SetNear( 0.01f );
				m_oModelRenderer.GetCamera()->SetFar( 10.0f );
				m_oFrame.AddChildTail( m_oModelRenderer );
				m_oModelRenderer.UpdateViewport();

				// The name of the reward
				if ( m_eType == TYPE_ITEM || m_eType == TYPE_TOKEN )
				{
					const TWCHAR* wszScreenName = m_pUnitType->GetScreenName();

					if ( wszScreenName )
					{
						m_oName.Create( pFont, pRootElement->GetWidth() );
						m_oName.SetText( wszScreenName );
						m_oName.SetScale( 1.5f );
						m_oName.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_MIDDLECENTER );
						m_oModelRenderer.AddChildTail( m_oName );
					}
				}
				else if ( m_eType == TYPE_MONEY )
				{
					ALocale::FormatTags( m_wszMoney, 64, 355, PPropertyValue( m_iMoney ) );

					m_oName.Create( pFont, pFont->GetTextWidth( m_wszMoney, 2.0f ) );
					m_oName.SetText( m_wszMoney );
					m_oName.SetScale( 1.5f );
					m_oName.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
					m_oModelRenderer.AddChildTail( m_oName );
				}
			}
		}

		// The title
		m_oTitle.Create( pFont, pFont->GetTextWidth( ALocaleManager::Interface()->GetString( 326 ), 1.0f ) );
		m_oTitle.SetText( ALocaleManager::Interface()->GetString( 326 ) );
		m_oTitle.SetScale( 1.5f );
		m_oTitle.SetColour( 0xffffc800 );
		m_oTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
		m_oFrame.AddChildTail( m_oTitle );

		m_oSubtitle.Create( pFont, pFont->GetTextWidth( ALocaleManager::Interface()->GetString( 354 ), 1.0f ) );
		m_oSubtitle.SetText( ALocaleManager::Interface()->GetString( 354 ) );
		m_oSubtitle.SetColour( 0xffffc800 );
		m_oSubtitle.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
		m_oTitle.AddChildTail( m_oSubtitle );

		m_oFrame.AddChildTail( m_oPressOK );
	}

	// The page of the unlocked picture
	m_oPicturePage.SetDimensions( m_oFrame.GetWidth(), m_oFrame.GetHeight() );
	m_oPicturePage.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	pRootElement->AddChildTail( m_oPicturePage );

	if ( m_ePage == PAGE_PICTURE )
	{
		AGameLoader::UnlockSimLoadingPicture( m_iPicture );

		TPString8 strPicture;
		AGUISystem::GetSingleton()->SetPicture( AGameLoader::GetSimLoadingPicture( strPicture, m_iPicture ) );

		m_oPicturePage.AddChildTail( m_oPressOK );
	}

	OnUpdate( 0.0f );
}

// $Barnyard: FUNCTION 00431ee0
void ABarnRewardState::OnActivate()
{
	AGameState::OnActivate();

	if ( m_ePage == PAGE_REWARD )
	{
		m_oFrame.Show();
		AGUISystem::GetSingleton()->SetPicture( TPS8( bkg_End_wood ) );
	}
	else if ( m_ePage == PAGE_PICTURE )
	{
		m_oPicturePage.Show();

		TPString8 strPicture;
		AGUISystem::GetSingleton()->SetPicture( AGameLoader::GetSimLoadingPicture( strPicture, m_iPicture ) );
	}
}

// $Barnyard: FUNCTION 004311f0
void ABarnRewardState::OnDeactivate()
{
	if ( m_ePage == PAGE_REWARD )
		m_oFrame.Hide();
	else if ( m_ePage == PAGE_PICTURE )
		m_oPicturePage.Hide();

	AGameState::OnDeactivate();
}
