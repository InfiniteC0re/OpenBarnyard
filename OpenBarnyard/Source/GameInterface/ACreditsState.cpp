#include "pch.h"
#include "ACreditsState.h"
#include "AFrontEndMainMenuState2.h"
#include "Assets/AAssetLoader.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2Style.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/AVOStream.h"
#include "SoundBank/music.h"
#include "Units/ASSeqController.h"

#include <File/TTRB.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Plugins/PPropertyParser/PBProperties.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004333a0, CreateObject 004335c0, CreateObjectInPlace 00433620 (the constructor is inlined in both)
TDEFINE_CLASS( ACreditsState );

TPSTRING8_DECLARE( DialogInputMap );
TPSTRING8_DECLARE( Title );
TPSTRING8_DECLARE( Name );
TPSTRING8_DECLARE( Names );
TPSTRING8_DECLARE( Image );

// $Barnyard: FUNCTION 004333b0
ACreditsState::Line::~Line()
{
	if ( wszText && !( eFlags & LINEFLAGS_LOCALISED ) )
		TFree( (void*)wszText );

	if ( pElement )
	{
		// Delete the children of the element, then the element itself
		for ( T2GUIElement* pChild = pElement->ChildRBegin(); pChild != TNULL; pChild = pElement->ChildRBegin() )
		{
			pChild->RemoveSelf();
			delete pChild;
		}

		pElement->RemoveSelf();
		delete pElement;
	}
}

// $Barnyard: FUNCTION 00433500
ACreditsState::ACreditsState( TBOOL a_bResetFade, TBOOL a_bBackground )
{
	m_pLines        = TNULL;
	m_pTitleFont    = TNULL;
	m_pNameFont     = TNULL;
	m_fNextLineTime = 0.0f;
	m_iMusicCue     = -1;
	m_bBackground   = a_bBackground;
	m_bResetFade    = a_bResetFade;
}

// $Barnyard: FUNCTION 004334a0
// $Barnyard: FUNCTION 004335a0
ACreditsState::~ACreditsState()
{
}

// $Barnyard: FUNCTION 00433450
TBOOL ACreditsState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand != AInputCommand_Quit && a_eInputCommand != AInputCommand_Cancel )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	Remove();
	return TTRUE;
}

// $Barnyard: FUNCTION 00433660
TBOOL ACreditsState::OnUpdate( TFLOAT a_fDeltaTime )
{
	AVOStream* pVOStream = AVOStream::GetSingleton();

	// The music starts once the voice-over is over
	if ( m_bVOStarted && m_iMusicCue == -1 && !pVOStream->IsPlaying() && !pVOStream->IsPlayRequested() )
		m_iMusicCue = ASoundManager::GetSingleton()->PlayCue( soundbank::MUSIC_CREDITS );

	if ( !m_bVOStarted && !pVOStream->IsPlayRequested() )
	{
		pVOStream->Stop();
		pVOStream->Play( 864, TFALSE );
		m_bVOStarted = TTRUE;
	}

	if ( m_llPendingLines.IsEmpty() && m_llVisibleLines.IsEmpty() )
	{
		// All lines have scrolled out
		Remove();
		return TTRUE;
	}

	m_fNextLineTime -= a_fDeltaTime;

	TFLOAT fScreenWidth, fScreenHeight;
	AGUI2::GetRootElement()->GetDimensions( fScreenWidth, fScreenHeight );

	if ( !m_llPendingLines.IsEmpty() && m_fNextLineTime <= 0.0f )
	{
		// Show the next line
		Line* pLine = m_llPendingLines.Head();

		pLine->pElement->SetColour( ( pLine->eFlags & LINEFLAGS_TITLE ) ? AGUI2STYLE_COLOR_SECONDARY : 0xffffffff );
		pLine->Remove();
		AGUI2::GetRootElement()->AddChildTail( *pLine->pElement );
		m_llVisibleLines.InsertTail( pLine );

		if ( !m_llPendingLines.IsEmpty() )
		{
			// The time until the next one depends on both kinds of lines
			LINEFLAGS eFlags     = pLine->eFlags;
			LINEFLAGS eNextFlags = m_llPendingLines.Head()->eFlags;

			if ( ( eFlags & LINEFLAGS_NAME ) && ( eNextFlags & ( LINEFLAGS_NAME | LINEFLAGS_LONGNAME ) ) )
				m_fNextLineTime = ms_fShortDelay;
			else if ( ( eFlags & LINEFLAGS_LONGNAME ) && ( eNextFlags & LINEFLAGS_NAME ) || ( eFlags & LINEFLAGS_LONGNAME ) && ( eNextFlags & LINEFLAGS_LONGNAME ) )
				m_fNextLineTime = ms_fShortDelay * 1.5f;
			else if ( ( eFlags & LINEFLAGS_NAME ) && ( eNextFlags & LINEFLAGS_TITLE ) )
				m_fNextLineTime = ms_fLongDelay * 1.5f;
			else if ( ( eFlags & LINEFLAGS_LONGNAME ) && ( eNextFlags & LINEFLAGS_TITLE ) )
				m_fNextLineTime = ms_fLongDelay * 2.5f;
			else if ( ( eFlags & LINEFLAGS_TITLE ) && ( eNextFlags & LINEFLAGS_TITLE ) )
				m_fNextLineTime = ms_fLongDelay;
			else if ( !( eFlags & LINEFLAGS_TITLE ) && !( eNextFlags & LINEFLAGS_IMAGE ) )
				m_fNextLineTime = ms_fShortDelay;
			else
				m_fNextLineTime = ms_fLongDelay;
		}
	}

	// Scroll the visible lines up
	for ( auto it = m_llVisibleLines.Begin(); it != m_llVisibleLines.End(); )
	{
		Line* pLine = it;
		it++;

		T2GUIElement*   pElement = pLine->pElement;
		const TVector2& vecPos   = pElement->GetTransform().GetTranslation();

		TFLOAT fY = vecPos.y - ms_fScrollRate * a_fDeltaTime;
		pElement->SetTranslation( vecPos.x, fY );

		TFLOAT fWidth, fHeight;
		pElement->GetDimensions( fWidth, fHeight );

		if ( fHeight + fScreenHeight <= TMath::Abs( fY ) )
		{
			// Out of the screen
			pLine->Remove();
			m_llFreeLines.InsertTail( pLine );
			pElement->RemoveSelf();
		}
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00433fd0
void ACreditsState::OnInsertion()
{
	m_bVOStarted = TFALSE;

	if ( m_bBackground )
	{
		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

		T2GUIElement* pRootElement = AGUI2::GetRootElement();
		m_oBackground.SetDimensions( fWidth, fHeight );
		m_oBackground.SetColour( 0xff000000 );
		pRootElement->AddChildTail( m_oBackground );
	}

	m_HUDParams.SetFlags( 0 );

	m_pLines = new Line[ MAX_NUM_LINES ];

	for ( TINT i = 0; i < MAX_NUM_LINES; i++ )
		m_llFreeLines.InsertTail( &m_pLines[ i ] );

	m_pTitleFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_pNameFont  = T2GUIFontManager::FindFont( "Rekord18" );
	Load( "credits" );
	m_fNextLineTime = 0.0f;

	if ( AFrontEndMainMenuState2::ms_bIsInserted )
	{
		AMusicManager::GetSingleton()->StopBackgroundMusic();
	}
	else
	{
		AVOStream::GetSingleton()->Close( TTRUE );
		AVOStream::GetSingleton()->Open( "data/sound/FSB/QuestVO.fsb", TFALSE, TFALSE );
	}

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00433930
void ACreditsState::OnRemoval()
{
	if ( m_bBackground )
		m_oBackground.RemoveSelf();

	m_llFreeLines.RemoveAll();
	m_llVisibleLines.RemoveAll();

	while ( !m_llPendingLines.IsEmpty() )
		m_llPendingLines.Head()->Remove();

	if ( m_pLines )
		delete[] m_pLines;

	ASoundManager::GetSingleton()->StopCue( m_iMusicCue );

	if ( AFrontEndMainMenuState2::ms_bIsInserted )
	{
		AMusicManager::GetSingleton()->PlayBackgroundMusic( soundbank::MUSIC_START_SCREEN );

		if ( AVOStream::GetSingleton()->IsPlaying() )
			AVOStream::GetSingleton()->Stop();
	}

	if ( m_bResetFade )
		ASSeqController::GetSingleton()->GetFade()->Reset( TTRUE );

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00433430
void ACreditsState::OnActivate()
{
	SetInputMap( TPS8( DialogInputMap ) );
	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 00433a10
TBOOL ACreditsState::Load( const TCHAR* a_szFileName )
{
	TFLOAT fScreenWidth, fScreenHeight;
	AGUI2::GetRootElement()->GetDimensions( fScreenWidth, fScreenHeight );

	const TFLOAT fMaxWidth = fScreenWidth * 0.9f;

	TTRB          oTRB;
	PBProperties* pProperties = TSTATICCAST( PBProperties, AAssetLoader::GetSymbolAddress( a_szFileName, "Main", AAssetType_AssetPack ) );

	if ( !pProperties )
	{
		TString8 strFileName;
		strFileName.Format( "Data/%s.trb", a_szFileName );

		if ( oTRB.Load( strFileName ) != TTRB::ERROR_OK )
			return TFALSE;

		pProperties = TSTATICCAST( PBProperties, oTRB.GetSymbolAddress( "Main" ) );
	}

	for ( const PBProperties::PBProperty* pProperty = pProperties->Begin(); pProperty != pProperties->End(); pProperty++ )
	{
		TPString8              strName( pProperty->GetName().GetString() );
		const PBPropertyValue* pValue = pProperty->GetValue();
		Line*                  pLine;

		if ( strName == TPS8( Title ) || strName == TPS8( Name ) )
		{
			pLine = m_llFreeLines.Tail();
			pLine->Remove();

			AGUI2TextBox* pTextBox = new AGUI2TextBox();
			pLine->pElement        = pTextBox;

			if ( pValue->GetType() == PBPropertyValue::Type::Int )
			{
				pLine->wszText = g_pLocaleManager->GetString( pValue->GetInteger() );
				pLine->eFlags |= LINEFLAGS_LOCALISED;
			}
			else
			{
				TPString8    strText = pValue->GetTPString8();
				const TCHAR* szText  = strText.GetString8().GetString();

				TWCHAR* wszText = TSTATICCAST( TWCHAR, TMalloc( TStringManager::String8Length( szText ) * 2 + 2 ) );
				TStringManager::StringCharToUnicode( wszText, szText, TStringManager::String8Length( szText ) + 1 );
				pLine->wszText = wszText;
			}

			if ( strName == TPS8( Title ) )
			{
				pTextBox->Create( m_pTitleFont, fMaxWidth );
				pLine->eFlags |= LINEFLAGS_TITLE;
			}
			else
			{
				pTextBox->Create( m_pNameFont, fMaxWidth );
				pLine->eFlags |= ( fMaxWidth > m_pNameFont->GetTextWidth( pLine->wszText ) ) ? LINEFLAGS_NAME : LINEFLAGS_LONGNAME;
			}

			pTextBox->SetText( pLine->wszText );
			pTextBox->SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
		}
		else if ( strName == TPS8( Names ) )
		{
			// A line for each name
			const PBPropertyValueArray* pNames = pValue->GetArray();

			for ( TUINT32 i = 0; i < pNames->GetSize(); i++ )
			{
				Line* pNameLine = m_llFreeLines.Tail();
				pNameLine->Remove();

				const TCHAR* szText  = pNames->GetValue( i )->GetString();
				TWCHAR*      wszText = TSTATICCAST( TWCHAR, TMalloc( TStringManager::String8Length( szText ) * 2 + 2 ) );
				TStringManager::StringCharToUnicode( wszText, szText, TStringManager::String8Length( szText ) + 1 );
				pNameLine->wszText = wszText;

				AGUI2TextBox* pTextBox = new AGUI2TextBox();
				pNameLine->pElement    = pTextBox;

				pTextBox->Create( m_pNameFont, fMaxWidth );
				pTextBox->SetText( pNameLine->wszText );
				pTextBox->SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );

				pNameLine->eFlags |= ( fMaxWidth > m_pNameFont->GetTextWidth( pNameLine->wszText ) ) ? LINEFLAGS_NAME : LINEFLAGS_LONGNAME;
				m_llPendingLines.InsertTail( pNameLine );
			}

			continue;
		}
		else if ( strName == TPS8( Image ) )
		{
			T2GUITextureSection* pTexSection = T2GUITextureSectionManager::GetTextureSection( pValue->GetString() );

			pLine = m_llFreeLines.Tail();
			pLine->Remove();

			T2GUIRectangle* pRectangle = new T2GUIRectangle();
			pLine->pElement            = pRectangle;

			pRectangle->SetDimensions( pTexSection->GetWidth(), pTexSection->GetHeight() );
			pRectangle->SetTextureSection( pTexSection );
			pRectangle->SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
			pLine->eFlags |= LINEFLAGS_IMAGE;
		}
		else
		{
			continue;
		}

		m_llPendingLines.InsertTail( pLine );
	}

	oTRB.Close();
	return TTRUE;
}
