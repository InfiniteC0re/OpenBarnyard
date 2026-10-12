#include "pch.h"
#include "AGameEventQuestElement.h"
#include "MiniGame/AMiniGameManager.h"
#include "MiniGame/AMiniGame.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Input/AInputHandler.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005cf540, CreateObject 005cfad0, CreateObjectInPlace 005cfb30
TDEFINE_CLASS( AGameEventQuestElement );

// 0x007753b0
static const TCHAR* s_aEndStates[] = { "win", "lose", "draw" };

// $Barnyard: FUNCTION 005cf760
// NOTE: retail leaves m_iWinPlayer and m_eEndState uninitialised
AGameEventQuestElement::AGameEventQuestElement()
    : m_iNumPlayers( 0 )
    , m_iWinPlayer( -1 )
    , m_eEndState( ENDSTATE_WIN )
    , m_iVariant( 0 )
    , m_strDescription( GetGlobalAllocator() )
    , m_iResultOffset( -1 )
{
}

// $Barnyard: FUNCTION 005cf7f0
// $Barnyard: FUNCTION 005cfb70
AGameEventQuestElement::~AGameEventQuestElement()
{
}

// $Barnyard: FUNCTION 005cfb90
TBOOL AGameEventQuestElement::Create( const PBProperties* a_pProperties, TINT* a_pDataOffset )
{
	m_iNumPlayers = 0;
	m_iVariant    = 0;

	const PBPropertyValue* pWinPlayer = TNULL;

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const TCHAR*           szName = it->GetName().GetString();
		const PBPropertyValue* pValue = it->GetValue();

		switch ( szName[ 0 ] )
		{
			case 'e':
				if ( TStringManager::String8CompareNoCase( szName, "endstate" ) == 0 )
					m_eEndState = GetEndState( pValue->GetTPString8() );
				break;
			case 'm':
				if ( TStringManager::String8CompareNoCase( szName, "minigame" ) == 0 )
					m_strMiniGame = pValue->GetTPString8();
				break;
			case 'p':
				if ( TStringManager::String8CompareNoCase( szName, "players" ) == 0 )
					ReadPlayers( a_pDataOffset, pValue->GetArray() );
				break;
			case 'v':
				if ( TStringManager::String8CompareNoCase( szName, "variant" ) == 0 )
					m_iVariant = pValue->GetInteger();
				break;
			case 'w':
				if ( TStringManager::String8CompareNoCase( szName, "winplayer" ) == 0 )
					pWinPlayer = pValue;
				break;
		}
	}

	m_iWinPlayer = -1;

	if ( pWinPlayer )
	{
		TPString8 strWinPlayer = pWinPlayer->GetTPString8();

		for ( TINT i = 0; i < m_iNumPlayers; i++ )
		{
			if ( strWinPlayer == m_astrPlayers[ i ] )
			{
				m_iWinPlayer = i;
				break;
			}
		}
	}

	m_iResultOffset = *a_pDataOffset;
	*a_pDataOffset += sizeof( PROGRESS );

	return AQuestElement::Create( a_pProperties, a_pDataOffset );
}

// $Barnyard: FUNCTION 005cf890
TBOOL AGameEventQuestElement::ReadPlayers( TINT* a_pDataOffset, const PBPropertyValueArray* a_pPlayers )
{
	m_iNumPlayers = a_pPlayers->GetSize();

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		m_astrPlayers[ i ] = a_pPlayers->GetValue( i )->GetTPString8();

		// NOTE: retail compares the name with "player" here and ignores the result
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005cf6f0
AGameEventQuestElement::ENDSTATE AGameEventQuestElement::GetEndState( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < TARRAYSIZE( s_aEndStates ); i++ )
	{
		if ( a_rcName == TPString8( s_aEndStates[ i ] ) )
			return ENDSTATE( i );
	}

	return ENDSTATE( -1 );
}

// $Barnyard: FUNCTION 005cf590
TBOOL AGameEventQuestElement::InitData( AInteractionActivityData* a_pData )
{
	*GetResult( a_pData ) = PROGRESS_RUNNING;
	return AQuestElement::InitData( a_pData );
}

// $Barnyard: FUNCTION 005cf550
TBOOL AGameEventQuestElement::GetDescription( const TWCHAR** a_ppOutText )
{
	// NOTE: retail passes the TCHAR string to the wide %s
	m_strDescription.Format( L"GameEventQuestElement: %s", m_strMiniGame.GetString() );
	*a_ppOutText = m_strDescription.GetString( 0 );

	return TTRUE;
}

// $Barnyard: FUNCTION 005cf5b0
void AGameEventQuestElement::Save( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData )
{
	a_pBuffer->WriteInt( *GetResult( a_pData ) );
	AQuestElement::Save( a_pBuffer, a_pData );
}

// $Barnyard: FUNCTION 005cf5e0
TBOOL AGameEventQuestElement::Load( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData )
{
	TINT iResult = -1;
	a_pBuffer->ReadInt( iResult );
	*GetResult( a_pData ) = iResult;

	return AQuestElement::Load( a_pBuffer, a_pData );
}

// $Barnyard: FUNCTION 005cf630
AQuestElement::PROGRESS AGameEventQuestElement::CheckProgress( AInteractionActivityData* a_pData )
{
	return *GetResult( a_pData );
}

// $Barnyard: FUNCTION 005cf650
void AGameEventQuestElement::OnMiniGameResult( AInteractionActivityData* a_pData, AMiniGameState::S1* a_pResults )
{
	PROGRESS* pResult = GetResult( a_pData );

	const TPString8& rcMiniGame = ( a_pResults->iMiniGameIndex >= 0 ) ?
	    AMiniGameManager::GetSingleton()->GetMiniGame( a_pResults->iMiniGameIndex )->GetName() :
	    a_pResults->strUnk;

	if ( TStringManager::String8CompareNoCase( m_strMiniGame.GetString(), rcMiniGame.GetString() ) == 0 )
	{
		if ( a_pResults->iResult != 0 )
		{
			*pResult                               = PROGRESS_FAILED;
			GetProgress( a_pData )->bFailedForGood = TTRUE;
		}
		else
		{
			*pResult = PROGRESS_COMPLETE;
		}
	}
}

// $Barnyard: FUNCTION 005cf970
void AGameEventQuestElement::OnMiniGameStart( const TPString8& a_rcName, AInteractionActivityData* a_pData )
{
	if ( TStringManager::String8CompareNoCase( a_rcName.GetString(), m_strMiniGame.GetString() ) != 0 )
		return;

	// Fill the player set of the minigame with the players of the element
	APlayerSet* pPlayerSet = APlayerManager::GetSingleton()->GetPlayerSet( 1 );
	pPlayerSet->Reset();

	for ( TINT i = 0; i < m_iNumPlayers - 1; i++ )
		pPlayerSet->AddAIPlayer();

	pPlayerSet->FinishUpAddingAIs();

	TINT iAIPlayer = 0;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		if ( TStringManager::String8Compare( m_astrPlayers[ i ].GetString(), "player" ) != 0 )
		{
			APlayerSlot* pSlot   = pPlayerSet->GetAIPlayerSlot( iAIPlayer );
			pSlot->m_strUnitType = m_astrPlayers[ i ];
			iAIPlayer++;
		}
		else
		{
			APlayerSlot* pSlot   = pPlayerSet->AddHumanPlayer( AInputHandler::GetSingleton()->GetMainController(), -1 );
			pSlot->m_strUnitType = m_astrPlayers[ i ];
		}
	}

	if ( m_iVariant > 0 )
		AMiniGameManager::GetSingleton()->SetMiniGameVariant( m_iVariant - 1 );
}
