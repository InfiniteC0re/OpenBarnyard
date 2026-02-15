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

	m_vecHumanSlots.Clear();
	m_iUnk1            = 2;
	m_iUnk2            = 2;
	m_iUnk3            = 2;
	m_iUnk4            = 2;
	m_iNumAIPlayers    = 0;
	m_iNumHumanPlayers = 0;
}

// $Barnyard: FUNCTION 00620310
TINT APlayerSet::AddAIPlayer()
{
	if ( m_iNumHumanPlayers + m_iNumAIPlayers < MAX_NUM_PLAYERS )
		m_iNumAIPlayers += 1;

	return m_iNumAIPlayers;
}

// $Barnyard: FUNCTION 006206d0
APlayerHumanSlot* APlayerSet::AddHumanPlayer( Toshi::TInputDeviceController* a_pController )
{
	// Adjust number of AI players
	if ( m_iNumHumanPlayers + m_iNumAIPlayers >= MAX_NUM_PLAYERS )
	{
		m_iNumAIPlayers -= 1;

		if ( m_iNumAIPlayers < 0 )
			m_iNumAIPlayers = 0;
	}

	// Find free slot
	decltype( m_vecHumanSlots )::Iterator itInsertBefore = m_vecHumanSlots.Begin();
	T2_FOREACH( m_vecHumanSlots, it )
	{
		if ( !it->m_bUsed )
		{
			itInsertBefore = it;
			break;
		}
	}

	APlayerHumanSlot oUsedSlot( a_pController, TTRUE );
	m_vecHumanSlots.InsertBefore( itInsertBefore, oUsedSlot );
	
	return &m_vecHumanSlots[ itInsertBefore.Index() ];
}

// $Barnyard: FUNCTION 00620400
TBOOL APlayerSet::IsPlayerSlotUsed( TINT a_iSlot ) const
{
	return m_vecHumanSlots[ a_iSlot ].m_bUsed & 1;
}

// $Barnyard: FUNCTION 006206d0
APlayerHumanSlot::APlayerHumanSlot( Toshi::TInputDeviceController* a_pController, TBOOL a_bUsed )
    : m_Unk1( 0 )
    , m_pController( a_pController )
    , m_bUsed( a_bUsed )
{
}

// $Barnyard: FUNCTION 006206e0
APlayerHumanSlot::APlayerHumanSlot( const APlayerHumanSlot& a_rcOther )
{
	m_Unk1         = a_rcOther.m_Unk1;
	m_pController  = a_rcOther.m_pController;
	m_strUnk       = a_rcOther.m_strUnk;
	m_bUsed = a_rcOther.m_bUsed;
}

// $Barnyard: FUNCTION 00620740
APlayerHumanSlot::~APlayerHumanSlot()
{
	Reset();
}

// $Barnyard: FUNCTION 006207c0
APlayerHumanSlot& APlayerHumanSlot::operator=( const APlayerHumanSlot& a_rcOther )
{
	Reset();

	m_Unk1         = a_rcOther.m_Unk1;
	m_pController  = a_rcOther.m_pController;
	m_strUnk       = a_rcOther.m_strUnk;
	m_bUsed = a_rcOther.m_bUsed;

	return *this;
}

void APlayerHumanSlot::Reset()
{
	TASSERT( TFALSE && "Destroy Unk1" );
	m_strUnk.SetPooledString( TNULL );
}

// T2Vector<APlayerHumanSlot>:
// $Barnyard: FUNCTION 006208e0
// $Barnyard: FUNCTION 006209c0
// $Barnyard: FUNCTION 00620b80
