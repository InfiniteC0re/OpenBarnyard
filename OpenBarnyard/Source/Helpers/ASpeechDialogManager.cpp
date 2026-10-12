#include "pch.h"
#include "ASpeechDialogManager.h"
#include "Units/AUnit.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054b750, CreateObject 0054bf60, CreateObjectInPlace 0054bfc0
TDEFINE_CLASS( ASpeechDialogManager );

static constexpr TFLOAT  SPEECH_SCALE_SPEED  = 1.0f / 0.3f; // 0x007246f8
static constexpr TFLOAT  SPEECH_TYPING_DELAY = 0.05f;
static constexpr TFLOAT  SPEECH_TYPED_DELAY  = 0.3f;
static constexpr TUINT32 SPEECH_NAME_COLOUR  = 0xff8de57c;

//-----------------------------------------------------------------------------
// ASpeechDialogManager::Speech
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0054ba30
ASpeechDialogManager::Speech::Speech()
    : uiId( 0 )
    , uiTextColour( 0xffffffff )
    , uiNameColour( 0xffffffff )
    , fScale( 1.0f )
    , wszText( TNULL )
    , wcSaved( 0 )
    , iChar( -1 )
    , fTime( 0.0f )
    , eState( STATE_NONE )
{
	pDialog = new ASpeechDialog();
	pDialog->SetOwner( this );
}

// $Barnyard: FUNCTION 0054bad0
ASpeechDialogManager::Speech::~Speech()
{
	if ( pDialog )
		delete pDialog;

	pUnit = TNULL;
}

// $Barnyard: FUNCTION 0054c000
void ASpeechDialogManager::Speech::Start( TBOOL a_bCreate, const TWCHAR* a_wszName, const TWCHAR* a_wszText, AUnit* a_pUnit, TUINT a_uiId, TBOOL a_bHidden )
{
	uiId = a_uiId;

	if ( a_pUnit )
	{
		pUnit = a_pUnit->GetSelfProxy();

		if ( !a_pUnit->IsPlayer() )
		{
			// NOTE: overwritten below unless the speech is hidden
			uiNameColour = 0xffffc800;
			uiTextColour = 0xffffffff;
		}
	}
	else
	{
		pUnit = TNULL;
	}

	pDialog->SetVisible( !a_bHidden );

	if ( a_bHidden )
	{
		eState = STATE_HIDDEN;
		return;
	}

	if ( a_bCreate )
	{
		// Open a new bubble
		eState = STATE_OPENING;
		pDialog->Create( "Rekord18", ASpeechDialogManager::GetSingleton()->m_pFont );
		pDialog->Show();
		fScale = 0.0f;
	}
	else
	{
		eState = STATE_TYPING;
		pDialog->Reset();
		fScale = 1.0f;
	}

	wszText      = const_cast<TWCHAR*>( a_wszText );
	wcSaved      = 0;
	iChar        = -1;
	fTime        = 0.0f;
	uiNameColour = SPEECH_NAME_COLOUR;
	uiTextColour = 0xffffffff;

	pDialog->SetText( TNULL, 0xffffffff );
	pDialog->HidePrompt();
	pDialog->SetName( a_wszName, uiNameColour );
	pDialog->SetScale( fScale );
}

// $Barnyard: FUNCTION 0054b760
TBOOL ASpeechDialogManager::Speech::Update( TFLOAT a_fDeltaTime )
{
	switch ( eState )
	{
		case STATE_NONE:
			return TFALSE;
		case STATE_OPENING:
			fScale += a_fDeltaTime * SPEECH_SCALE_SPEED;

			if ( fScale >= 1.0f )
			{
				fScale = 1.0f;
				eState = STATE_SHOWN;
			}

			pDialog->SetScale( fScale );
			break;
		case STATE_TYPING:
			fTime += a_fDeltaTime;

			if ( fTime > SPEECH_TYPING_DELAY )
			{
				fTime -= SPEECH_TYPING_DELAY;

				// Restore the hidden character and hide the next one
				if ( iChar >= 0 )
					wszText[ iChar ] = wcSaved;

				iChar++;

				if ( wszText[ iChar ] == 0 )
				{
					// The whole text is shown
					iChar = -1;
					pDialog->ShowPrompt();
					fTime  = 0.0f;
					eState = STATE_TYPED;
				}
				else
				{
					wcSaved          = wszText[ iChar ];
					wszText[ iChar ] = 0;
				}

				pDialog->SetText( wszText, uiTextColour );
			}

			break;
		case STATE_TYPED:
			fTime += a_fDeltaTime;

			if ( fTime > SPEECH_TYPED_DELAY )
				eState = STATE_SHOWN;

			break;
		case STATE_CLOSING:
			fScale -= a_fDeltaTime * SPEECH_SCALE_SPEED;

			if ( fScale <= 0.0f )
			{
				fScale = 0.0f;
				eState = STATE_NONE;
			}

			pDialog->SetScale( fScale );
			break;
	}

	if ( eState != STATE_HIDDEN )
		pDialog->Update( a_fDeltaTime );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054b8d0
void ASpeechDialogManager::Speech::FinishTyping()
{
	if ( eState == STATE_TYPING )
	{
		if ( iChar >= 0 )
			wszText[ iChar ] = wcSaved;

		iChar = -1;
		pDialog->ShowPrompt();
		fTime  = 0.0f;
		eState = STATE_TYPED;
		pDialog->SetText( wszText, uiTextColour );
	}
}

//-----------------------------------------------------------------------------
// ASpeechDialogManager::Choice
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0054bb40
ASpeechDialogManager::Choice::Choice()
    : fScale( 1.0f )
    , eState( STATE_NONE )
{
	pDialog = new ASpeechChoiceDialog();
}

// $Barnyard: FUNCTION 0054bbd0
ASpeechDialogManager::Choice::~Choice()
{
	if ( pDialog )
		delete pDialog;

	Remove();
}

// $Barnyard: FUNCTION 0054b920
TBOOL ASpeechDialogManager::Choice::Update( TFLOAT a_fDeltaTime )
{
	switch ( eState )
	{
		case STATE_NONE:
			return TFALSE;
		case STATE_OPENING:
			fScale += a_fDeltaTime * SPEECH_SCALE_SPEED;

			if ( fScale >= 1.0f )
			{
				fScale = 1.0f;
				eState = STATE_SHOWN;
			}

			pDialog->SetScale( fScale );
			break;
		case STATE_CLOSING:
			fScale -= a_fDeltaTime * SPEECH_SCALE_SPEED;

			if ( fScale <= 0.0f )
			{
				fScale = 0.0f;
				eState = STATE_NONE;
				pDialog->Update( a_fDeltaTime );
				return TTRUE;
			}

			pDialog->SetScale( fScale );
			break;
	}

	pDialog->Update( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 0054b9d0
void ASpeechDialogManager::Choice::SetSelection( TINT a_iSelection )
{
	pDialog->SetSelection( a_iSelection );
}

//-----------------------------------------------------------------------------
// ASpeechDialogManager
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0054bc30
ASpeechDialogManager::ASpeechDialogManager()
    : m_pSpeeches( TNULL )
    , m_bChoiceShown( TFALSE )
{
	// The text uses Rekord18 with more space between the lines
	m_pFont = T2GUIFontManager::FindFont( "Rekord18" )->Clone();
	m_pFont->GetFontDef()->uiLinesMargin += 8;

	m_pChoice = new Choice();
}

// $Barnyard: FUNCTION 0054c1b0
// $Barnyard: FUNCTION 0054c4e0
ASpeechDialogManager::~ASpeechDialogManager()
{
	EndAllSpeeches();
	m_llUsedSpeeches.RemoveAll();
	m_llFreeSpeeches.RemoveAll();

	if ( m_pSpeeches )
		delete[] m_pSpeeches;

	if ( m_bChoiceShown && m_pChoice->eState != Choice::STATE_CLOSING && m_pChoice->eState != Choice::STATE_NONE )
		m_pChoice->eState = Choice::STATE_CLOSING;

	if ( m_pChoice )
		delete m_pChoice;

	if ( m_pFont )
		delete m_pFont;
}

// $Barnyard: FUNCTION 0054c290
TBOOL ASpeechDialogManager::Reset()
{
	EndAllSpeeches();

	while ( !m_llUsedSpeeches.IsEmpty() )
		FreeSpeech( m_llUsedSpeeches.Begin() );

	CloseChoice();

	m_pChoice->pDialog->Hide();
	m_pChoice->eState = Choice::STATE_NONE;
	m_bChoiceShown    = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 0054bd00
TBOOL ASpeechDialogManager::OnCreate()
{
	m_pSpeeches = new Speech[ NUM_SPEECHES ];
	m_llFreeSpeeches.InsertTail( m_pSpeeches );

	return TTask::OnCreate();
}

// $Barnyard: FUNCTION 0054c330
TBOOL ASpeechDialogManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_llUsedSpeeches.Begin(); it != m_llUsedSpeeches.End(); )
	{
		Speech* pSpeech = it;
		it++;

		if ( !pSpeech->Update( a_fDeltaTime ) )
			FreeSpeech( pSpeech );
	}

	if ( m_bChoiceShown && !m_pChoice->Update( a_fDeltaTime ) )
	{
		m_pChoice->pDialog->Hide();
		m_pChoice->eState = Choice::STATE_NONE;
		m_bChoiceShown    = TFALSE;
	}

	return TTask::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0054c400
TFLOAT ASpeechDialogManager::StartSpeech( const TWCHAR* a_wszName, const TWCHAR* a_wszText, AUnit* a_pUnit, TUINT a_uiId, TBOOL a_bHidden )
{
	Speech* pSpeech = FindReusableSpeech();
	TBOOL   bCreate = TFALSE;

	if ( !pSpeech )
	{
		if ( m_llFreeSpeeches.IsEmpty() )
			return 0.0f;

		pSpeech = m_llFreeSpeeches.Begin();
		pSpeech->Remove();
		m_llUsedSpeeches.InsertTail( pSpeech );
		bCreate = TTRUE;
	}

	pSpeech->Start( bCreate, a_wszName, a_wszText, a_pUnit, a_uiId, a_bHidden );
	return 100000.0f;
}

// $Barnyard: FUNCTION 0054c480
TBOOL ASpeechDialogManager::PrepareSpeech()
{
	if ( m_llUsedSpeeches.IsEmpty() )
		StartSpeech( TNULL, TNULL, TNULL, 0, TFALSE );

	for ( auto it = m_llUsedSpeeches.Begin(); it != m_llUsedSpeeches.End(); it++ )
	{
		if ( it->eState == Speech::STATE_OPENING )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0054bd90
ASpeechDialogManager::Speech* ASpeechDialogManager::FindSpeech( TUINT a_uiId )
{
	for ( auto it = m_llUsedSpeeches.Begin(); it != m_llUsedSpeeches.End(); it++ )
	{
		if ( it->uiId == a_uiId )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0054bde0
ASpeechDialogManager::Speech* ASpeechDialogManager::FindReusableSpeech()
{
	for ( auto it = m_llUsedSpeeches.Begin(); it != m_llUsedSpeeches.End(); it++ )
	{
		if ( it->eState == Speech::STATE_SHOWN || it->eState == Speech::STATE_HIDDEN )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0054be30
TBOOL ASpeechDialogManager::IsSpeechFinished( TUINT a_uiId )
{
	Speech* pSpeech = FindSpeech( a_uiId );

	if ( !pSpeech )
		return TTRUE;

	return pSpeech->eState == Speech::STATE_SHOWN || pSpeech->eState == Speech::STATE_HIDDEN;
}

// $Barnyard: FUNCTION 0054beb0
TBOOL ASpeechDialogManager::SkipSpeech( TUINT a_uiId )
{
	Speech* pSpeech = FindSpeech( a_uiId );

	if ( !pSpeech )
		return TTRUE;

	if ( pSpeech->eState == Speech::STATE_TYPING )
	{
		pSpeech->FinishTyping();
		return TFALSE;
	}

	return pSpeech->eState == Speech::STATE_SHOWN || pSpeech->eState == Speech::STATE_HIDDEN;
}

// $Barnyard: FUNCTION 0054be60
void ASpeechDialogManager::StopSpeech( TUINT a_uiId )
{
	Speech* pSpeech = FindSpeech( a_uiId );

	if ( pSpeech )
	{
		pSpeech->uiId    = 0;
		pSpeech->pUnit   = TNULL;
		pSpeech->eState  = Speech::STATE_SHOWN;
		pSpeech->fTime   = 0.0f;
		pSpeech->iChar   = -1;
		pSpeech->wcSaved = 0;
		pSpeech->pDialog->Reset();
	}
}

// $Barnyard: FUNCTION 0054bef0
TBOOL ASpeechDialogManager::EndAllSpeeches()
{
	if ( m_llUsedSpeeches.IsEmpty() )
		return TTRUE;

	for ( auto it = m_llUsedSpeeches.Begin(); it != m_llUsedSpeeches.End(); it++ )
	{
		if ( it->eState == Speech::STATE_HIDDEN )
			it->eState = Speech::STATE_NONE;
		else if ( it->eState != Speech::STATE_CLOSING && it->eState != Speech::STATE_NONE )
			it->eState = Speech::STATE_CLOSING;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0054b9f0
void ASpeechDialogManager::CloseChoice()
{
	if ( m_bChoiceShown && m_pChoice->eState != Choice::STATE_CLOSING && m_pChoice->eState != Choice::STATE_NONE )
		m_pChoice->eState = Choice::STATE_CLOSING;
}

void ASpeechDialogManager::FreeSpeech( Speech* a_pSpeech )
{
	a_pSpeech->pDialog->Hide();
	a_pSpeech->uiId    = 0;
	a_pSpeech->pUnit   = TNULL;
	a_pSpeech->eState  = Speech::STATE_NONE;
	a_pSpeech->fTime   = 0.0f;
	a_pSpeech->iChar   = -1;
	a_pSpeech->wcSaved = 0;
	a_pSpeech->Remove();
	m_llFreeSpeeches.InsertTail( a_pSpeech );
}
