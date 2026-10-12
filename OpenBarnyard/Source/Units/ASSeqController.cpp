#include "pch.h"
#include "ASSeqController.h"
#include "ASSeq.h"
#include "FSM/AMethodPointer.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"
#include "GameInterface/ASSeqState.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00629b00, CreateObject 0062a9d0, CreateObjectInPlace 0062aa30
TDEFINE_CLASS_INIT( ASSeqController );

AFunctionTable ASSeqController::ms_oFunctionTable( &TGetClass( ASSeqController ), TNULL );

// FSM fallback: ASSeqController natives always run on the singleton.
// $Barnyard: FUNCTION 00629d00
static TObject* __stdcall ASSeqControllerFunctionFallback( void* a_pUnk1, void* a_pUnk2 )
{
	return ASSeqController::GetSingleton();
}

// $Barnyard: FUNCTION 0062b0e0
void ASSeqController::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ASSeqControllerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer5ArgsRT<TBOOL, ASSeqController, TPString8, TPString8, TPString8, TBOOL, TBOOL>::Create( "StartSSeqState", &ASSeqController::StartSSeqState ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASSeqController>::Create( "IsSeqPlaying", &ASSeqController::IsSeqPlaying ) );
}

// $Barnyard: FUNCTION 00629b10
void ASSeqController::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0062a350
ASSeqController::ASSeqController()
    : m_vecLoadedScripts( GetGlobalAllocator(), 64, 64 )
    , m_vecPlayingScripts( GetGlobalAllocator(), 64, 64 )
    , m_iNumActors( 0 )
    , m_ppActors( TNULL )
    , m_pActorFlags( TNULL )
    , m_bStopAllScripts( TFALSE )
    , m_vecUpdatedActors( GetGlobalAllocator(), 64, 64 )
    , m_pFade( TNULL )
{
	m_pFade = new ASSeqFade();
}

// $Barnyard: FUNCTION 0062b130
// $Barnyard: FUNCTION 0062aa70
ASSeqController::~ASSeqController()
{
	Reset();

	if ( m_pFade )
		delete m_pFade;

	m_pFade = TNULL;
}

// $Barnyard: FUNCTION 0062a4a0
TBOOL ASSeqController::Reset()
{
	m_oTextBox.Hide();

	for ( auto it = m_vecPlayingScripts.Begin(); it != m_vecPlayingScripts.End(); it++ )
	{
		if ( it.Value() )
			delete it.Value();
	}

	m_vecPlayingScripts.Clear();

	for ( auto it = m_vecLoadedScripts.Begin(); it != m_vecLoadedScripts.End(); it++ )
	{
		if ( it.Value() )
			delete it.Value();
	}

	m_vecLoadedScripts.Clear();

	// NOTE: retail frees the actors that are updated every frame
	while ( m_vecUpdatedActors.Size() != 0 )
	{
		ASSeqActorInterface* pActor = m_vecUpdatedActors[ 0 ];
		m_vecUpdatedActors.Erase( m_vecUpdatedActors.Begin() );
		TFree( pActor );
	}

	if ( m_ppActors )
	{
		TFree( m_ppActors );
		m_ppActors = TNULL;
	}

	if ( m_pActorFlags )
	{
		TFree( m_pActorFlags );
		m_pActorFlags = TNULL;
	}

	m_iNumActors = 0;
	m_pFade->Reset( TFALSE );
	m_bStopAllScripts = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 00629d10
TBOOL ASSeqController::OnCreate()
{
	T2GUIFont* pFont  = T2GUIFontManager::FindFont( "Rekord26" );
	TFLOAT     fWidth = pFont->GetTextWidth( ALocaleManager::Interface()->GetString( 463 ), 1.0f );

	m_oTextBox.Create( pFont, fWidth );
	m_oTextBox.SetText( ALocaleManager::Interface()->GetString( 474 ) );
	m_oTextBox.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	AGameStateController::GetSingleton()->GetOverlayElement().AddChildTail( m_oTextBox );

	m_oSubTextBox.Create( pFont, fWidth );
	m_oSubTextBox.SetText( ALocaleManager::Interface()->GetString( 483 ) );
	m_oSubTextBox.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oSubTextBox.SetTranslation( 0.0f, m_oTextBox.GetHeight() * 0.25f );
	m_oTextBox.AddChildHead( m_oSubTextBox );

	m_oTextBox.Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 0062ab70
TBOOL ASSeqController::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bStopAllScripts )
	{
		StopAllScripts();
		m_bStopAllScripts = TFALSE;
	}

	if ( m_vecPlayingScripts.Size() != 0 )
	{
		for ( TINT i = 0; i < m_iNumActors; i++ )
		{
			if ( m_pActorFlags[ i ] & ACTORFLAGS_UPDATE )
				m_ppActors[ i ]->UpdateActor( a_fDeltaTime );
		}

		// The first sequence that is over goes back to the loaded ones
		for ( auto it = m_vecPlayingScripts.Begin(); it != m_vecPlayingScripts.End(); it++ )
		{
			ASSeq* pSSeq = it.Value();
			pSSeq->Update( a_fDeltaTime );

			if ( !pSSeq->IsPlaying() )
			{
				m_vecPlayingScripts.Erase( it );
				m_vecLoadedScripts.PushBack( pSSeq );
				break;
			}
		}
	}

	if ( m_pFade )
		m_pFade->Update( a_fDeltaTime );

	return TTask::OnUpdate( a_fDeltaTime );
}

// Shows the chapter title again if a sequence is playing
// $Barnyard: FUNCTION 0062a480
void ASSeqController::OnActivate()
{
	if ( m_vecPlayingScripts.Size() != 0 )
		m_oTextBox.Show();

	TTask::OnActivate();
}

// $Barnyard: FUNCTION 00629b60
void ASSeqController::OnDeactivate()
{
	m_oTextBox.Hide();
	TTask::OnDeactivate();
}

// $Barnyard: FUNCTION 00629b20
void ASSeqController::OnDestroy()
{
	m_oTextBox.RemoveSelf();
	m_oSubTextBox.RemoveSelf();
}

// $Barnyard: FUNCTION 00629b70
void ASSeqController::CreateActors( TINT a_iNumActors )
{
	m_iNumActors = a_iNumActors;
	m_ppActors   = TSTATICCAST( ASSeqActorInterface*, TMalloc( a_iNumActors * sizeof( ASSeqActorInterface* ) ) );

	for ( TINT i = 0; i < m_iNumActors; i++ )
		m_ppActors[ i ] = TNULL;

	// NOTE: the flags are left uninitialised, ASSeq clears them before playing
	m_pActorFlags = TSTATICCAST( ACTORFLAGS, TMalloc( m_iNumActors * sizeof( ACTORFLAGS ) ) );
}

// $Barnyard: FUNCTION 00629bd0
void ASSeqController::DestroyActors()
{
	TFree( m_ppActors );
	m_ppActors = TNULL;

	TFree( m_pActorFlags );
	m_pActorFlags = TNULL;

	m_iNumActors = 0;
}

// $Barnyard: FUNCTION 00629c10
void ASSeqController::ClearActorFlags()
{
	for ( TINT i = 0; i < m_iNumActors; i++ )
		m_pActorFlags[ i ] = ACTORFLAGS_NONE;
}

// $Barnyard: FUNCTION 00629c40
void ASSeqController::SetActorFlag( TINT a_iActor, TINT a_iFlag )
{
	m_pActorFlags[ a_iActor ] |= 1 << a_iFlag;
}

// $Barnyard: FUNCTION 00629cb0
void ASSeqController::SetChapterTitleAlpha( TFLOAT a_fAlpha )
{
	TMath::Clip( a_fAlpha, 0.0f, 1.0f );

	m_oTextBox.SetAlpha( a_fAlpha );
	m_oSubTextBox.SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 00629e10
void ASSeqController::SetChapterTitle( const TCHAR* a_szScriptName )
{
	TINT iTitle;
	TINT iSubTitle;

	if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS01_start" ) == 0 )
	{
		iTitle    = 474;
		iSubTitle = 483;
	}
	else if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS02_rac" ) == 0 )
	{
		iTitle    = 475;
		iSubTitle = 484;
	}
	else if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS03" ) == 0 )
	{
		iTitle    = 476;
		iSubTitle = 485;
	}
	else if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS04" ) == 0 )
	{
		iTitle    = 477;
		iSubTitle = 486;
	}
	else if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS05" ) == 0 )
	{
		iTitle    = 478;
		iSubTitle = 487;
	}
	else if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS06" ) == 0 )
	{
		iTitle    = 479;
		iSubTitle = 488;
	}
	else if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS07a" ) == 0 )
	{
		iTitle    = 480;
		iSubTitle = 489;
	}
	else if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS08" ) == 0 )
	{
		iTitle    = 481;
		iSubTitle = 490;
	}
	else if ( TStringManager::String8CompareNoCase( a_szScriptName, "SS09" ) == 0 )
	{
		iTitle    = 482;
		iSubTitle = 491;
	}
	else
	{
		return;
	}

	m_oSubTextBox.SetScale( 1.0f );
	m_oTextBox.SetText( ALocaleManager::Interface()->GetString( iTitle ) );
	m_oSubTextBox.SetText( ALocaleManager::Interface()->GetString( iSubTitle ) );
	m_oTextBox.SetAlpha( 1.0f );
	m_oSubTextBox.SetAlpha( 1.0f );

	// Shrink the sub title if it doesn't fit the screen
	TFLOAT fWidth       = m_oSubTextBox.GetWidth();
	TFLOAT fScreenWidth = AGUI2::GetRootElement()->GetWidth();

	if ( fWidth > fScreenWidth && fScreenWidth > 0.0f )
		m_oSubTextBox.SetScale( 1.0f - ( fWidth - fScreenWidth ) / fScreenWidth );
}

// $Barnyard: FUNCTION 0062a020
TBOOL ASSeqController::IsSeqPlaying()
{
	return m_vecPlayingScripts.Size() != 0;
}

// $Barnyard: FUNCTION 0062a030
void ASSeqController::ShowChapterTitle( TBOOL a_bShow )
{
	if ( a_bShow == TTRUE )
	{
		m_oTextBox.Show();
		m_oTextBox.RemoveSelf();
		AGameStateController::GetSingleton()->GetOverlayElement().AddChildTail( m_oTextBox );
		m_oTextBox.SetAlpha( 1.0f );
		m_oSubTextBox.SetAlpha( 1.0f );
	}
	else
	{
		m_oTextBox.Hide();
	}
}

// $Barnyard: FUNCTION 0062a5e0
void ASSeqController::StopScript( const TCHAR* a_szName, const TCHAR* a_szFileName )
{
	m_oTextBox.Hide();

	for ( auto it = m_vecPlayingScripts.Begin(); it != m_vecPlayingScripts.End(); )
	{
		ASSeq* pSSeq = it.Value();

		if ( TStringManager::String8CompareNoCase( pSSeq->GetName(), a_szName ) == 0 &&
		     ( a_szFileName == TNULL || TStringManager::String8CompareNoCase( pSSeq->GetFileName(), a_szFileName ) == 0 ) )
		{
			pSSeq->Stop();
			m_vecPlayingScripts.Erase( it );
			m_vecLoadedScripts.PushBack( pSSeq );

			// NOTE: retail skips the sequence that follows the stopped one
			if ( it == m_vecPlayingScripts.End() )
				break;
		}

		it++;
	}
}

// $Barnyard: FUNCTION 0062a760
void ASSeqController::StopAllScripts()
{
	m_oTextBox.Hide();

	while ( m_vecPlayingScripts.Size() != 0 )
		StopScript( m_vecPlayingScripts.Back().Value()->GetName(), TNULL );
}

// $Barnyard: FUNCTION 0062a7a0
const TCHAR* ASSeqController::GetCurrentScriptName()
{
	if ( m_vecPlayingScripts.Size() == 0 )
		return TNULL;

	return m_vecPlayingScripts.Begin().Value()->GetName();
}

// $Barnyard: FUNCTION 0062a7d0
ASSeq* ASSeqController::FindScript( T2DynamicVector<ASSeq*>& a_rScripts, const TCHAR* a_szName, const TCHAR* a_szFileName )
{
	for ( auto it = a_rScripts.Begin(); it != a_rScripts.End(); it++ )
	{
		ASSeq* pSSeq = it.Value();

		if ( TStringManager::String8CompareNoCase( pSSeq->GetName(), a_szName ) == 0 &&
		     ( a_szFileName == TNULL || TStringManager::String8CompareNoCase( pSSeq->GetFileName(), a_szFileName ) == 0 ) )
		{
			return pSSeq;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0062a860
ASSeq* ASSeqController::FindLoadedScript( const TCHAR* a_szFileName )
{
	for ( auto it = m_vecLoadedScripts.Begin(); it != m_vecLoadedScripts.End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it.Value()->GetFileName(), a_szFileName ) == 0 )
			return it.Value();
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0062a8d0
void ASSeqController::SetActor( TINT a_iActor, ASSeqActorInterface* a_pActor, TBOOL a_bUpdate )
{
	m_ppActors[ a_iActor ] = a_pActor;

	if ( a_bUpdate )
	{
		if ( m_vecUpdatedActors.Find( m_ppActors[ a_iActor ] ) == m_vecUpdatedActors.End() )
			m_vecUpdatedActors.PushBack( m_ppActors[ a_iActor ] );
	}
}

// $Barnyard: FUNCTION 0062a960
void ASSeqController::UpdateActorFlags( TINT a_iActor, TBOOL a_bUnused )
{
	for ( auto it = m_vecUpdatedActors.Begin(); it != m_vecUpdatedActors.End(); it++ )
	{
		if ( it.Value() == m_ppActors[ a_iActor ] )
			m_pActorFlags[ a_iActor ] |= ACTORFLAGS_UPDATE;
	}
}

// $Barnyard: FUNCTION 0062ac70
TBOOL ASSeqController::LoadScript( const TCHAR* a_szFileName, const TVector4& a_rcPosition )
{
	if ( FindLoadedScript( a_szFileName ) )
		return TTRUE;

	ASSeq* pSSeq = new ASSeq( this );

	if ( pSSeq->Load( a_szFileName, a_rcPosition ) )
	{
		m_vecLoadedScripts.PushBack( pSSeq );
		return TTRUE;
	}

	if ( pSSeq )
		delete pSSeq;

	return TFALSE;
}

// $Barnyard: FUNCTION 0062ad40
TBOOL ASSeqController::StartScript( const TCHAR* a_szName, const TCHAR* a_szFileName, TFLOAT a_fFadeTime, TFLOAT a_fSyncSpeed )
{
	ASSeq* pSSeq = FindScript( m_vecLoadedScripts, a_szName, a_szFileName );

	if ( pSSeq )
	{
		m_vecLoadedScripts.FindAndErase( pSSeq );
		m_vecPlayingScripts.PushBack( pSSeq );
		pSSeq->Start( a_fFadeTime, a_fSyncSpeed );

		return TTRUE;
	}

	if ( !FindScript( m_vecPlayingScripts, a_szName, a_szFileName ) )
		m_oTextBox.Hide();

	return TFALSE;
}

// $Barnyard: FUNCTION 0062ae10
ASSeq* ASSeqController::FindScript( const TCHAR* a_szName, const TCHAR* a_szFileName )
{
	ASSeq* pSSeq = FindScript( m_vecLoadedScripts, a_szName, a_szFileName );

	if ( !pSSeq )
		pSSeq = FindScript( m_vecPlayingScripts, a_szName, a_szFileName );

	return pSSeq;
}

// $Barnyard: FUNCTION 0062ae40
TBOOL ASSeqController::StartSSeqState( TPString8 a_strScript, TPString8 a_strUnk1, TPString8 a_strUnk2, TBOOL a_bUnk1, TBOOL a_bUnk2 )
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( a_strScript.GetString(), a_strUnk1.GetString(), a_strUnk2.GetString(), TVector4::VEC_ZERO, 0, TTRUE, a_bUnk1, a_bUnk2, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);

	return TFALSE;
}
