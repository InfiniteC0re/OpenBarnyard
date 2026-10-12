#include "pch.h"
#include "AHighScoreManager.h"
#include "AGameLoader.h"
#include "ASimPersistData.h"
#include "Units/AInteractionManager.h"
#include "Player/APlayerProfileManager.h"
#include "Locale/ALocaleManager.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0041bd20, CreateObject 0041cc60, CreateObjectInPlace 0041ccc0
TDEFINE_CLASS( AHighScoreManager );

TSTATICASSERT( sizeof( AHighScoreManager ) == 0x36ac );
TSTATICASSERT( AHighScoreManager::MAX_RECORDS * sizeof( AHighScoreRecord ) == 0x17e8 );

TPSTRING8_DECLARE( pip );

// The default scores of a record in highscore.trb ("GameData")
struct HighScoreGameData
{
	struct Score
	{
		TINT   iPlayer;
		TFLOAT fScore;
	};

	TINT  iRecord;
	TINT  eType;
	Score aScores[ AHighScoreRecord::NUM_SCORES ];
};

TSTATICASSERT( sizeof( HighScoreGameData ) == 48 );

// The names of a record in highscore.trb ("GameName")
struct HighScoreGameName
{
	TINT iNameId;
	TINT iShortNameId;
};

// $Barnyard: FUNCTION 0041cb70
AHighScoreManager::AHighScoreManager()
{
	if ( !ms_bLoaded )
		ScoreReset( TTRUE );

	m_bChartEnabled      = TFALSE;
	ms_iLastRank         = -1;
	ms_iLastPreviousRank = -1;
	ms_iPendingRecord    = -1;
	ms_iLastRecord       = -1;
	ms_iShowRecord       = -1;
	ms_bShowChart        = TFALSE;
}

// $Barnyard: FUNCTION 0041c3c0
// $Barnyard: FUNCTION 0041c570
AHighScoreManager::~AHighScoreManager()
{
}

// $Barnyard: FUNCTION 0041bd30
TBOOL AHighScoreManager::OnCreate()
{
	m_oChart.Create();

	// RETURNS_TRUE 0046cde0
	return TTRUE;
}

// $Barnyard: FUNCTION 0041c110
TBOOL AHighScoreManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Adds the pending score after a second
	if ( ms_iPendingRecord != -1 )
	{
		m_fPendingTimer -= a_fDeltaTime;

		if ( m_fPendingTimer < 0.0f )
		{
			AddScore( ms_iPendingRecord, m_fPendingScore, TTRUE, ms_iPendingPlayer, TFALSE );
			ms_iPendingRecord = -1;
		}
	}

	if ( ms_bShowChart && !m_bChartEnabled )
		ms_bShowChart = TFALSE;

	if ( m_bChartEnabled )
	{
		// Hides the chart after a while
		if ( m_oChart.IsVisible() )
		{
			m_fChartTimer -= a_fDeltaTime;

			if ( m_fChartTimer < 0.0f )
				m_oChart.SetVisible( TFALSE );
		}

		if ( ms_bShowChart )
		{
			m_oChart.Show( &ms_aRecords[ ms_iShowRecord ], TFALSE );
			m_oChart.SetVisible( TTRUE );
			ms_bShowChart = TFALSE;
			m_fChartTimer = 5.0f;
		}
	}

	m_oChart.Update( a_fDeltaTime );

	// RETURNS_TRUE 00413920
	return TTRUE;
}

// $Barnyard: FUNCTION 0041c050
void AHighScoreManager::SetChartEnabled( TBOOL a_bEnabled )
{
	m_bChartEnabled = a_bEnabled;

	if ( !a_bEnabled )
		m_oChart.SetVisible( TFALSE );
}

// $Barnyard: FUNCTION 0041c080
TBOOL AHighScoreManager::ShowLastRecord( TINT& a_rOutRank, TBOOL a_bClearRank )
{
	if ( ms_iLastRecord == -1 )
		return TFALSE;

	a_rOutRank = ms_aRecords[ ms_iLastRecord ].iLastRank;

	m_oChart.Show( &ms_aRecords[ ms_iLastRecord ], TFALSE );
	m_oChart.SetVisible( TTRUE );

	if ( a_bClearRank )
		ms_aRecords[ ms_iLastRecord ].iLastRank = -1;

	return TTRUE;
}

// $Barnyard: FUNCTION 0041c0f0
void AHighScoreManager::HideChart()
{
	m_oChart.Clear();
	m_oChart.SetVisible( TFALSE );
}

// $Barnyard: FUNCTION 0041c420
void AHighScoreManager::SetPendingScore( TINT a_iRecord, TFLOAT a_fScore, TINT a_iPlayer )
{
	if ( ms_iPendingRecord != a_iRecord || ms_iPendingRecord == -1 )
	{
		// Another record, adds the score of the previous one
		if ( ms_iPendingRecord != a_iRecord && ms_iPendingRecord != -1 )
			AddScore( ms_iPendingRecord, m_fPendingScore, TTRUE, ms_iPendingPlayer, TFALSE );

		ms_iPendingRecord = a_iRecord;
		m_fPendingScore   = a_fScore;
		ms_iPendingPlayer = a_iPlayer;
	}

	AHighScoreRecord::TYPE eType = ms_aRecords[ ms_iPendingRecord ].eType;

	// The score got worse, adds the best one
	TBOOL bWorse;

	if ( eType == AHighScoreRecord::TYPE_INTEGER_LOW || eType == AHighScoreRecord::TYPE_DECIMAL_LOW || eType == AHighScoreRecord::TYPE_TIME_LOW )
		bWorse = a_fScore > m_fPendingScore;
	else
		bWorse = a_fScore < m_fPendingScore;

	if ( bWorse )
	{
		AddScore( ms_iPendingRecord, m_fPendingScore, TTRUE, ms_iPendingPlayer, TFALSE );
		ms_iPendingRecord = -1;
		return;
	}

	m_fPendingTimer = 1.0f;
	m_fPendingScore = a_fScore;
}

// $Barnyard: FUNCTION 0041c810
void AHighScoreManager::ClearLastRecord()
{
	if ( ms_iLastRecord != -1 )
		ms_aRecords[ ms_iLastRecord ].iLastRank = -1;

	ms_iLastRecord    = -1;
	ms_iPendingRecord = -1;
	ms_iShowRecord    = -1;
	ms_bShowChart     = TFALSE;

	AInteractionManager::GetSingleton()->SetPersistentLatch( TPS8D( "pip_feedback" ), 0 );
	AInteractionManager::GetSingleton()->SetPersistentLatch( TPS8D( "pip_champion" ), 0 );
}

// $Barnyard: FUNCTION 0041cc10
void AHighScoreManager::ShowChampionChart()
{
	UpdateChampionChart();

	ms_aRecords[ RECORD_STARS ].iLastRank = -1;
	m_oChart.Show( &ms_aRecords[ RECORD_STARS ], TTRUE );
	m_oChart.SetVisible( TTRUE );
}

// $Barnyard: FUNCTION 0041cc40
AHighScoreManager::Record* AHighScoreManager::GetChampionChart( Record* a_pOutRecord )
{
	UpdateChampionChart();
	*a_pOutRecord = ms_aRecords[ RECORD_STARS ];

	return a_pOutRecord;
}

// $Barnyard: FUNCTION 0041c590
TBOOL AHighScoreManager::ScoreReset( TBOOL a_bInitialising )
{
	ms_bLoaded = TTRUE;

	for ( TINT i = NUM_RECORDS - 1; i >= 0; i-- )
	{
		Record& rRecord = ms_aRecords[ i ];

		rRecord.iIndex       = i;
		rRecord.eType        = AHighScoreRecord::TYPE_INTEGER_HIGH;
		rRecord.iLastRank    = -1;
		rRecord.iNameId      = -1;
		rRecord.iShortNameId = -1;
		rRecord.iHidden      = 0;
		rRecord.bPlayed      = TFALSE;

		for ( TINT k = NUM_SCORES - 1; k >= 0; k-- )
		{
			rRecord.aScores[ k ]  = 0.0f;
			rRecord.aPlayers[ k ] = PLAYER_1;
		}
	}

	TTRB oTRB;
	oTRB.Load( "Data\\MinigameData\\HighScoreChart\\highscore.trb" );

	TINT*              pNumRecords = oTRB.CastSymbol<TINT>( "NumberOfRecords" );
	HighScoreGameData* pGameData   = oTRB.CastSymbol<HighScoreGameData>( "GameData" );

	// The default scores
	for ( TINT i = *pNumRecords - 1; i >= 0; i-- )
	{
		HighScoreGameData& rData           = pGameData[ i ];
		ms_aRecords[ rData.iRecord ].eType = rData.eType;

		for ( TINT k = NUM_SCORES - 1; k >= 0; k-- )
			AddScore( rData.iRecord, rData.aScores[ k ].fScore, TTRUE, rData.aScores[ k ].iPlayer, TFALSE );

		ms_aRecords[ i ].iLastRank = -1;
	}

	// The grades
	TFLOAT( *pGrades )
	[NUM_GRADES] = oTRB.CastSymbol<TFLOAT[ NUM_GRADES ]>( "Grades" );

	for ( TINT i = *pNumRecords - 1; i >= 0; i-- )
	{
		for ( TINT k = 0; k < NUM_GRADES; k++ )
			ms_aGradeLimits[ i ][ k ] = pGrades[ i ][ k ];
	}

	// The names
	HighScoreGameName* pGameNames = oTRB.CastSymbol<HighScoreGameName>( "GameName" );

	for ( TINT i = *pNumRecords - 1; i >= 0; i-- )
	{
		ms_aRecords[ i ].iNameId      = pGameNames[ i ].iNameId;
		ms_aRecords[ i ].iShortNameId = pGameNames[ i ].iShortNameId;
	}

	// The flags
	TINT* pGameFlags = oTRB.CastSymbol<TINT>( "GameFlags" );

	for ( TINT i = *pNumRecords - 1; i >= 0; i-- )
		ms_aRecords[ i ].iHidden = pGameFlags[ i ];

	oTRB.Close();

	ms_bShowChart  = TFALSE;
	ms_iLastRecord = -1;
	ms_iShowRecord = -1;

	if ( !a_bInitialising )
	{
		AInteractionManager::GetSingleton()->SetPersistentLatch( TPS8D( "pip_feedback" ), 0 );
		AInteractionManager::GetSingleton()->SetPersistentLatch( TPS8D( "pip_champion" ), 0 );
	}

	return a_bInitialising;
}

// $Barnyard: FUNCTION 0041bd50
TBOOL AHighScoreManager::AddScore( TINT a_iRecord, TFLOAT a_fScore, TBOOL a_bAdd, TINT a_iPlayer, TBOOL a_bUnique )
{
	ms_iLastRecord  = a_iRecord;
	Record& rRecord = ms_aRecords[ a_iRecord ];

	if ( a_iPlayer == PLAYER_1 )
		rRecord.bPlayed = TTRUE;

	if ( !a_bAdd )
	{
		rRecord.iLastRank = -1;
		return TFALSE;
	}

	// Finds the rank of the score, the empty scores are 0
	TINT                   iRank = -1;
	AHighScoreRecord::TYPE eType = rRecord.eType;

	if ( eType == AHighScoreRecord::TYPE_INTEGER_LOW || eType == AHighScoreRecord::TYPE_DECIMAL_LOW || eType == AHighScoreRecord::TYPE_TIME_LOW )
	{
		for ( TINT i = 0; i < NUM_SCORES; i++ )
		{
			if ( !( a_fScore >= rRecord.aScores[ i ] && 0.0f != rRecord.aScores[ i ] ) )
			{
				iRank = i;
				break;
			}
		}
	}
	else
	{
		for ( TINT i = 0; i < NUM_SCORES; i++ )
		{
			if ( !( a_fScore <= rRecord.aScores[ i ] && 0.0f != rRecord.aScores[ i ] ) )
			{
				iRank = i;
				break;
			}
		}
	}

	rRecord.iLastRank = iRank;

	if ( iRank == -1 )
		return TFALSE;

	// The rank the player already has
	TINT iPreviousRank = -1;

	for ( TINT i = 0; i < NUM_SCORES; i++ )
	{
		if ( a_iPlayer == rRecord.aPlayers[ i ] )
		{
			iPreviousRank = i;
			break;
		}
	}

	if ( iPreviousRank == -1 || !a_bUnique )
	{
		// Inserts the score
		for ( TINT i = NUM_SCORES - 1; i > iRank; i-- )
		{
			rRecord.aScores[ i ]  = rRecord.aScores[ i - 1 ];
			rRecord.aPlayers[ i ] = rRecord.aPlayers[ i - 1 ];
		}

		rRecord.aScores[ iRank ]  = a_fScore;
		rRecord.aPlayers[ iRank ] = a_iPlayer;
		ms_bShowChart             = TTRUE;

		if ( ms_iPendingRecord != -1 )
		{
			ms_iLastRank         = iRank;
			ms_iLastPreviousRank = iPreviousRank;
		}

		ms_iShowRecord = a_iRecord;
		return TTRUE;
	}

	if ( iPreviousRank == iRank )
	{
		// Same rank, the score is better
		rRecord.aScores[ iPreviousRank ] = a_fScore;
		ms_iShowRecord                   = a_iRecord;
		rRecord.aPlayers[ iRank ]        = a_iPlayer;
		ms_bShowChart                    = TTRUE;
		return TTRUE;
	}

	if ( iPreviousRank < iRank )
	{
		// The player already has a better score
		ms_iShowRecord = a_iRecord;
		ms_bShowChart  = TFALSE;
		return TTRUE;
	}

	// Moves the score of the player up
	for ( TINT i = iPreviousRank; i > iRank; i-- )
	{
		rRecord.aScores[ i ]  = rRecord.aScores[ i - 1 ];
		rRecord.aPlayers[ i ] = rRecord.aPlayers[ i - 1 ];
	}

	ms_iShowRecord            = a_iRecord;
	rRecord.aPlayers[ iRank ] = a_iPlayer;
	rRecord.aScores[ iRank ]  = a_fScore;
	ms_bShowChart             = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 0041c2a0
AHighScoreManager::GRADE AHighScoreManager::GetGrade( TINT a_iRecord, TFLOAT a_fScore )
{
	AHighScoreRecord::TYPE eType = ms_aRecords[ a_iRecord ].eType;
	TINT                   iGrade;

	if ( eType == AHighScoreRecord::TYPE_INTEGER_LOW || eType == AHighScoreRecord::TYPE_DECIMAL_LOW || eType == AHighScoreRecord::TYPE_TIME_LOW )
	{
		for ( iGrade = 0; a_fScore > ms_aGradeLimits[ a_iRecord ][ iGrade ]; )
		{
			if ( ++iGrade >= NUM_GRADES )
				return GRADE_F;
		}
	}
	else
	{
		for ( iGrade = 0; a_fScore < ms_aGradeLimits[ a_iRecord ][ iGrade ]; )
		{
			if ( ++iGrade >= NUM_GRADES )
				return GRADE_F;
		}
	}

	return iGrade;
}

// $Barnyard: FUNCTION 0041c550
AHighScoreManager::GRADE AHighScoreManager::GetLastRecordGrade( TFLOAT a_fScore )
{
	if ( ms_iLastRecord >= 0 )
		return GetGrade( ms_iLastRecord, a_fScore );

	return GRADE_F;
}

// $Barnyard: FUNCTION 0041c330
TINT AHighScoreManager::GetNumStars( TINT a_iPlayer )
{
	TINT iNumStars = 0;

	for ( TINT i = NUM_RECORDS - 1; i >= 0; i-- )
	{
		if ( i == RECORD_STARS || ms_aRecords[ i ].iHidden != 0 )
			continue;

		for ( TINT k = NUM_SCORES - 1; k >= 0; k-- )
		{
			if ( a_iPlayer == ms_aRecords[ i ].aPlayers[ k ] && GetGrade( i, ms_aRecords[ i ].aScores[ k ] ) == GRADE_STAR )
			{
				iNumStars++;
				break;
			}
		}
	}

	return iNumStars;
}

// $Barnyard: FUNCTION 0041c500
TBOOL AHighScoreManager::HasHumanStar( TINT a_iRecord )
{
	for ( TINT i = 0; i < NUM_SCORES; i++ )
	{
		if ( ms_aRecords[ a_iRecord ].aPlayers[ i ] < 0 && GetGrade( a_iRecord, ms_aRecords[ a_iRecord ].aScores[ i ] ) == GRADE_STAR )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0041c240
TINT AHighScoreManager::PopLastRank()
{
	TINT iRank = ms_iLastRank;

	if ( iRank < 0 )
		return -1;

	ms_iLastRank = -1;
	return iRank;
}

// $Barnyard: FUNCTION 0041c260
TINT AHighScoreManager::PopLastPreviousRank()
{
	TINT iRank = ms_iLastPreviousRank;

	if ( iRank < 0 )
		return -1;

	ms_iLastPreviousRank = -1;
	return iRank;
}

// $Barnyard: FUNCTION 0041c8c0
void AHighScoreManager::UpdateChampionChart()
{
	// Adds the number of stars of every player of the charts
	for ( TINT i = RECORD_STARS - 1; i >= 0; i-- )
	{
		for ( TINT k = NUM_SCORES - 1; k >= 0; k-- )
		{
			TINT iPlayer = ms_aRecords[ i ].aPlayers[ k ];
			AddScore( RECORD_STARS, TFLOAT( GetNumStars( iPlayer ) ), TTRUE, iPlayer, TTRUE );
		}
	}

	// The second player goes after the characters it's tied with
	// NOTE: unrolled in the original
	Record& rChart = ms_aRecords[ RECORD_STARS ];

	for ( TINT i = 0; i < NUM_SCORES - 1; i++ )
	{
		if ( rChart.aPlayers[ i ] == PLAYER_2 && rChart.aScores[ i ] == rChart.aScores[ i + 1 ] )
		{
			TFLOAT fScore            = rChart.aScores[ i + 1 ];
			rChart.aScores[ i + 1 ]  = rChart.aScores[ i ];
			rChart.aPlayers[ i ]     = rChart.aPlayers[ i + 1 ];
			rChart.aScores[ i ]      = fScore;
			rChart.aPlayers[ i + 1 ] = PLAYER_2;
		}
	}

	// The rank of the main player
	TINT iRank = -1;

	for ( TINT i = 0; i < NUM_SCORES; i++ )
	{
		if ( rChart.aPlayers[ i ] < 0 )
		{
			iRank = i + 1;
			break;
		}
	}

	if ( iRank != ASimPersistData::GetSingleton()->GetRank() || AGameLoader::IsLoadingSavedGame() )
	{
		if ( iRank != -1 )
		{
			if ( iRank == 1 )
			{
				// Pip congratulates the new champion
				if ( AInteractionManager::GetSingleton()->GetPersistentLatch( "pip_champion" ) == 0 )
				{
					if ( !AGameLoader::IsBarnExploreMiniGame() )
						APlayerProfileManager::GetSingleton()->GetMainProfile()->AddPendingItem( ALocaleManager::Interface()->GetString( 3336 ), TPS8( pip ), 3336 );

					AInteractionManager::GetSingleton()->SetPersistentLatch( TPS8D( "pip_champion" ), 1 );
				}
			}
			else if ( !AGameLoader::IsLoadingSavedGame() )
			{
				AInteractionManager::GetSingleton()->SetPersistentLatch( TPS8D( "pip_feedback" ), 1 );
			}
		}

		ASimPersistData::GetSingleton()->SetRank( iRank );
	}
}

// $Barnyard: FUNCTION 0041c220
TBOOL AHighScoreManager::Save( void* a_pBuffer, TINT a_iSize )
{
	TUtil::MemCopy( a_pBuffer, ms_aRecords, 0x17e8 );
	return TTRUE;
}

// $Barnyard: FUNCTION 0041c8a0
TBOOL AHighScoreManager::Load( const void* a_pBuffer, TINT a_iSize )
{
	ScoreReset( TFALSE );
	TUtil::MemCopy( ms_aRecords, a_pBuffer, 0x17e8 );
	return TTRUE;
}
