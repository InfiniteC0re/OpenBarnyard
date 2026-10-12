#include "pch.h"
#include "AMiniGameResultRequirement.h"
#include "Helpers/AHighScoreManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ad6a0, CreateObject 005adac0
// NOTE: GetClass 005ad6a0, CreateObject 005adac0, CreateObjectInPlace 005adb20
TDEFINE_CLASS( AMiniGameResultRequirement );

// $Barnyard: FUNCTION 005ad740
AMiniGameResultRequirement::AMiniGameResultRequirement()
    : m_uiFlags( FLAGS_NONE )
    , m_iMinGamesPlayed( 0 )
    , m_iMaxGamesPlayed( -1 )
    , m_fWinRatio( 0.0f )
    , m_oCompare( AInteractionComparison::OPERATOR_ALL )
    , m_iGameIndex( -1 )
    , m_fHighScore( 0.0f )
{
}

// $Barnyard: FUNCTION 005ad7b0
// $Barnyard: FUNCTION 005adb60
AMiniGameResultRequirement::~AMiniGameResultRequirement()
{
}

// $Barnyard: FUNCTION 005ad820
TBOOL AMiniGameResultRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strCompare;

	if ( a_pProperties->GetOptionalPropertyValue( m_strMiniGame, TPS8D( "minigame" ) ) )
	{
		m_uiFlags |= FLAGS_MINIGAME;

		// NOTE: "mingamesplayed" is misspelled in retail too
		a_pProperties->GetOptionalPropertyValue( m_iMinGamesPlayed, TPS8D( "mingamesplayed" ) );
		a_pProperties->GetOptionalPropertyValue( m_iMaxGamesPlayed, TPS8D( "maxgamesplayed" ) );
		a_pProperties->GetOptionalFloatOrIntValue( m_fWinRatio, TPS8D( "winratio" ) );
		a_pProperties->GetOptionalPropertyValue( strCompare, TPS8D( "compare" ) );
	}
	else
	{
		m_uiFlags |= FLAGS_HIGHSCORE;

		a_pProperties->GetOptionalPropertyValue( m_iGameIndex, TPS8D( "gameindex" ) );
		a_pProperties->GetOptionalFloatOrIntValue( m_fHighScore, TPS8D( "highscoreval" ) );
		a_pProperties->GetOptionalPropertyValue( strCompare, TPS8D( "compare" ) );
	}

	m_oCompare.SetOperator( strCompare );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005ad6b0
void AMiniGameResultRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TFLOAT fWeight = 0.0f;

	if ( !( m_uiFlags & FLAGS_MINIGAME ) && ( m_uiFlags & FLAGS_HIGHSCORE ) )
	{
		// The best score of the human players in the record
		AHighScoreManager::Record* pRecord = AHighScoreManager::GetRecord( m_iGameIndex );

		TFLOAT fScore = 0.0f;

		for ( TINT i = 0; i < AHighScoreManager::NUM_SCORES; i++ )
		{
			if ( pRecord->aPlayers[ i ] < 0 )
			{
				fScore = pRecord->aScores[ i ];
				break;
			}
		}

		if ( m_oCompare.Compare( fScore, m_fHighScore ) )
			fWeight = 1.0f;
	}

	// NOTE: only the weight is set
	if ( m_bInvert == TTRUE )
		fWeight = 1.0f - fWeight;

	m_oRoles.fWeight = fWeight;
}
