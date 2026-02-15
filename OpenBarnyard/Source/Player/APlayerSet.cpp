#include "pch.h"
#include "APlayerSet.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00620bc0
APlayerSet::APlayerSet()
{
}

// $Barnyard: FUNCTION 00620c30
// $Barnyard: FUNCTION 00620a90
APlayerSet::~APlayerSet()
{
	Reset();
}

// $Barnyard: FUNCTION 00620b00
void APlayerSet::Reset()
{
	m_strUnk.SetPooledString( TNULL );

	m_vecPlayerSlots.Clear();
	m_aPlayerTeams[ 0 ] = 2;
	m_aPlayerTeams[ 1 ] = 2;
	m_aPlayerTeams[ 2 ] = 2;
	m_aPlayerTeams[ 3 ] = 2;
	m_iNumAIPlayers     = 0;
	m_iNumHumanPlayers  = 0;
}

// $Barnyard: FUNCTION 00620310
TINT APlayerSet::AddAIPlayer()
{
	if ( GetTotalNumPlayers() < MAX_NUM_PLAYERS )
		m_iNumAIPlayers += 1;

	return m_iNumAIPlayers;
}

// $Barnyard: FUNCTION 006206d0
APlayerSlot* APlayerSet::AddHumanPlayer( Toshi::TInputDeviceController* a_pController )
{
	// Adjust number of AI players
	if ( GetTotalNumPlayers() >= MAX_NUM_PLAYERS )
	{
		m_iNumAIPlayers -= 1;

		if ( m_iNumAIPlayers < 0 )
		{
			TASSERT( TFALSE );
			m_iNumAIPlayers = 0;
		}
	}

	// Find free slot
	auto itInsertBefore = m_vecPlayerSlots.Begin();
	T2_FOREACH( m_vecPlayerSlots, it )
	{
		if ( it->m_uiFlags == APlayerSlot::FLAGS_HUMAN )
		{
			itInsertBefore = it;
			break;
		}
	}

	m_vecPlayerSlots.InsertBefore( itInsertBefore, APlayerSlot( a_pController, APlayerSlot::FLAGS_HUMAN ) );
	return &m_vecPlayerSlots[ itInsertBefore.Index() ];
}

// $Barnyard: FUNCTION 006205f0
APlayerSlot* APlayerSet::GetPlayerSlot( TINT a_iSlot )
{
	TASSERT( a_iSlot < m_vecPlayerSlots.Size() );
	return &m_vecPlayerSlots[ a_iSlot ];
}

// $Barnyard: FUNCTION 006205f0
void APlayerSet::MakeTeamsFair()
{
	if ( m_aPlayerTeams[ 0 ] == APLAYERTEAM_NONE ) return;
	
	const TINT iTotalNumPlayers = GetTotalNumPlayers();
	if ( iTotalNumPlayers > 1 )
	{
		// Check if there's at least one player in different team
		for ( TINT i = 1; i < iTotalNumPlayers; i++ )
		{
			if ( m_aPlayerTeams[ 0 ] != m_aPlayerTeams[ i ] ) break;

			// Every player is in a single team
			if ( i + 1 >= iTotalNumPlayers ) return;
		}

		if ( m_iNumHumanPlayers < iTotalNumPlayers )
		{
			// There are some AI players, so need to fill flags for them
			for ( TINT i = m_iNumHumanPlayers; i < iTotalNumPlayers; i++ )
				m_aPlayerTeams[ i ] = APLAYERTEAM_NONE;
		}

		// Count Team 1 players
		TINT iNumTeam1 = 0;
		for ( TINT i = 0; i < iTotalNumPlayers; i++ )
		{
			if ( m_aPlayerTeams[ i ] == APLAYERTEAM_TEAM1 ) iNumTeam1++;
		}

		// Count Team 2 players
		TINT iNumTeam2 = 0;
		for ( TINT i = 0; i < iTotalNumPlayers; i++ )
		{
			if ( m_aPlayerTeams[ i ] == APLAYERTEAM_TEAM2 ) iNumTeam2++;
		}

		// Balance teams
		for ( TINT i = 0; i < iTotalNumPlayers; i++ )
		{
			if ( m_aPlayerTeams[ i ] == APLAYERTEAM_NONE )
			{
				if ( iNumTeam1 < iNumTeam2 )
				{
					m_aPlayerTeams[ i ] = APLAYERTEAM_TEAM1;
					iNumTeam1 += 1;
				}
				else
				{
					m_aPlayerTeams[ i ] = APLAYERTEAM_TEAM2;
					iNumTeam2 += 1;
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 00620820
void APlayerSet::FinishUpAddingAIs()
{
	// Count all player slots without the AI flag
	TINT iCurrentNumAIPlayers = 0;
	for ( TINT i = 0; i < m_vecPlayerSlots.Size(); i++ )
	{
		if ( m_vecPlayerSlots[ i ].m_uiFlags & APlayerSlot::FLAGS_AI )
			iCurrentNumAIPlayers += 1;
	}

	// Add missing AI players
	while ( iCurrentNumAIPlayers < m_iNumAIPlayers )
	{
		m_vecPlayerSlots.PushBack( APlayerSlot( TNULL, APlayerSlot::FLAGS_AI ) );
		iCurrentNumAIPlayers += 1;
	}
}

// $Barnyard: FUNCTION 00620400
TBOOL APlayerSet::IsPlayerSlotUsedByHuman( TINT a_iSlot ) const
{
	return m_vecPlayerSlots[ a_iSlot ].m_uiFlags & APlayerSlot::FLAGS_HUMAN;
}

TBOOL APlayerSet::IsPlayerSlotUsedByAI( TINT a_iSlot ) const
{
	return m_vecPlayerSlots[ a_iSlot ].m_uiFlags & APlayerSlot::FLAGS_AI;
}

// $Barnyard: FUNCTION 006206d0
APlayerSlot::APlayerSlot( Toshi::TInputDeviceController* a_pController, FLAGS a_uiFlags )
    : m_Unk1( 0 )
    , m_pController( a_pController )
    , m_uiFlags( a_uiFlags )
{
}

// $Barnyard: FUNCTION 006206e0
APlayerSlot::APlayerSlot( const APlayerSlot& a_rcOther )
{
	m_Unk1        = a_rcOther.m_Unk1;
	m_pController = a_rcOther.m_pController;
	m_strUnk      = a_rcOther.m_strUnk;
	m_uiFlags     = a_rcOther.m_uiFlags;
}

// $Barnyard: FUNCTION 00620740
APlayerSlot::~APlayerSlot()
{
	Reset();
}

// $Barnyard: FUNCTION 006207c0
APlayerSlot& APlayerSlot::operator=( const APlayerSlot& a_rcOther )
{
	Reset();

	m_Unk1        = a_rcOther.m_Unk1;
	m_pController = a_rcOther.m_pController;
	m_strUnk      = a_rcOther.m_strUnk;
	m_uiFlags     = a_rcOther.m_uiFlags;

	return *this;
}

void APlayerSlot::Reset()
{
	TASSERT( TFALSE && "Destroy Unk1" );
	m_strUnk.SetPooledString( TNULL );
}

// T2Vector<APlayerHumanSlot>:
// $Barnyard: FUNCTION 006208e0
// $Barnyard: FUNCTION 006209c0
// $Barnyard: FUNCTION 00620b80
