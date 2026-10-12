#pragma once
#include <Math/TRandom.h>

//-----------------------------------------------------------------------------
// Purpose: Picks one of the added values at random, a value with a bigger
// weight being more likely to be picked.
// NOTE: the name is invented. Retail keeps the entries in a T2Vector (iterated
// with 004ab160/00575090); the entries are constructed (zeroed) like here.
// Used by ATapperMiniGameState::PickSpawnLane and ARegrowthManager::CrateSchema::PickRandom.
//-----------------------------------------------------------------------------
template <class T, TINT MAX_ENTRIES>
class AWeightedRandom
{
public:
	struct Entry
	{
		T      oValue  = T();
		TFLOAT fWeight = 0.0f;
	};

public:
	AWeightedRandom( Toshi::TRandom* a_pRandom )
	    : m_pRandom( a_pRandom )
	    , m_fTotalWeight( 0.0f )
	    , m_iNumEntries( 0 )
	{}

	void Add( const T& a_rcValue, TFLOAT a_fWeight )
	{
		m_fTotalWeight += a_fWeight;
		m_aEntries[ m_iNumEntries ].oValue    = a_rcValue;
		m_aEntries[ m_iNumEntries++ ].fWeight = a_fWeight;
	}

	TINT GetNumEntries() const { return m_iNumEntries; }

	// Returns the picked entry, TNULL if there's nothing to pick from
	// $Barnyard: FUNCTION 004e1fe0
	Entry* Pick()
	{
		if ( m_iNumEntries == 0 || m_fTotalWeight <= 1.1920929e-7f )
			return TNULL;

		// NOTE: retail calls a TRandom helper (004010d0) returning a float from 0 to m_fTotalWeight
		TFLOAT fRandom = ( m_fTotalWeight - 1.1920929e-7f ) * TFLOAT( m_pRandom->RandRaw() >> 1 ) * 4.6566129e-10f;
		TFLOAT fSum    = 0.0f;

		for ( TINT i = 0; i < m_iNumEntries; i++ )
		{
			fSum += m_aEntries[ i ].fWeight;

			if ( fSum > fRandom )
				return &m_aEntries[ i ];
		}

		return TNULL;
	}

private:
	Toshi::TRandom* m_pRandom;                 // 0x0
	TFLOAT          m_fTotalWeight;            // 0x4
	TINT            m_iNumEntries;             // 0x8
	Entry           m_aEntries[ MAX_ENTRIES ]; // 0xc
};
