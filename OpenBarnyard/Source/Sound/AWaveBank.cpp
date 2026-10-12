#include "pch.h"
#include "AWaveBank.h"
#include "ASoundManager.h"

#include <ToshiTools/T2DynamicVector.h>

#include <Plugins/PPropertyParser/PBProperties.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005dcd90
AWaveBank::AWaveBank( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strPath, TBOOL a_bSetFrequency )
{
	m_iNumWaves = 0;
	m_iNumLoads = 0;
	m_strBank   = a_strBank;
	m_strPath   = a_strPath;
	m_pWaves    = TNULL;

	m_FileHandle    = TNULL;
	m_bSetFrequency = a_bSetFrequency;

	ASoundManager::ms_WaveBanks.Insert( a_strBank, this );
}

// $Barnyard: FUNCTION 005dc200
AWaveSampleHandle AWaveBank::GetWaveSample( TINT a_iWaveIndex, TINT a_iUnused ) const
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	return m_pWaves[ a_iWaveIndex ].pSampleHandle;
}

void AWaveBank::ResetWave( TINT a_iWaveIndex )
{
}

// $Barnyard: FUNCTION 005dce30
// $Barnyard: FUNCTION 005dcf20
AWaveBank::~AWaveBank()
{
	ASoundManager::ms_WaveBanks.FindAndRemove( m_strBank );
	delete[] m_pWaves;
}

// $Barnyard: FUNCTION 005dc260
AWave* AWaveBank::GetWave( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	return &m_pWaves[ a_iWaveIndex ];
}

// $Barnyard: FUNCTION 005dc130
AWaveSampleHandle AWaveBank::GetWaveSampleHandle( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	return m_pWaves[ a_iWaveIndex ].pSampleHandle;
}

// $Barnyard: FUNCTION 005dc1e0
TFLOAT AWaveBank::GetWaveLength( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	return m_pWaves[ a_iWaveIndex ].fLength;
}

// $Barnyard: FUNCTION 005dc1a0
TINT AWaveBank::GetWaveId( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	return m_pWaves[ a_iWaveIndex ].iIndex;
}

// $Barnyard: FUNCTION 005dc1c0
TINT AWaveBank::GetWaveFrequency( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	return m_pWaves[ a_iWaveIndex ].iFrequency;
}

// $Barnyard: FUNCTION 005dc150
AWaveSampleHandle AWaveBank::CreateWaveSample( TINT a_iWaveIndex, TINT a_iUnused )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );

	if ( m_pWaves[ a_iWaveIndex ].pSampleHandle == TNULL )
	{
		m_pWaves[ a_iWaveIndex ].pSampleHandle = GetWaveSample( a_iWaveIndex, a_iUnused );
	}

	return m_pWaves[ a_iWaveIndex ].pSampleHandle;
}

// $Barnyard: FUNCTION 005dc220
void AWaveBank::SetWaveFlag1( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	m_pWaves[ a_iWaveIndex ].iFlags &= ~AWave::FLAGS_3D;
	m_pWaves[ a_iWaveIndex ].iFlags |= AWave::FLAGS_2D;
}

// $Barnyard: FUNCTION 005dc240
void AWaveBank::SetWaveFlag2( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	m_pWaves[ a_iWaveIndex ].iFlags &= ~AWave::FLAGS_2D;
	m_pWaves[ a_iWaveIndex ].iFlags |= AWave::FLAGS_3D;
}

// $Barnyard: FUNCTION 005dc300
void AWaveBank::DestroyAll()
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail iterates the map while ~AWaveBank removes the bank from it (reads the successor of a freed node), take the first one instead
	while ( ASoundManager::ms_WaveBanks.Size() != 0 )
	{
		AWaveBank* pWaveBank = ASoundManager::ms_WaveBanks.Begin()->GetSecond();
		pWaveBank->Unload();
		delete pWaveBank;
	}
#else
	for ( auto it = ASoundManager::ms_WaveBanks.Begin(); it != ASoundManager::ms_WaveBanks.End(); it++ )
	{
		AWaveBank* pWaveBank = it->GetSecond();
		pWaveBank->Unload();
		delete pWaveBank;
	}
#endif
}

// $Barnyard: FUNCTION 005dc390
void AWaveBank::ParseWavesData( const PBProperties* a_pBankProperties, TUINT a_uiForcedFlags )
{
	TVALIDPTR( a_pBankProperties );

	m_iNumWaves = 0;

	T2DynamicVector<AWave> vecWaves( GetGlobalAllocator(), 128, 128 );

	for ( TUINT i = 0; i < a_pBankProperties->GetPropertyCount(); i++ )
	{
		const PBProperties::PBProperty* pProperty = a_pBankProperties->GetPropertyByIndex( i );

		// Skip any properties other than Wave
		if ( pProperty->GetName().GetString()[ 0 ] != 'W' ) continue;

		TINT   iFrequency = 22050;
		TFLOAT fLength    = 0.0f;
		TUINT  uiFlags    = a_uiForcedFlags;

		const PBProperties* pWaveProperties = pProperty->GetValue()->GetProperties();

		// Parse flags
		const PBPropertyValue* pFlagsProperty = pWaveProperties->GetProperty( "flags" );
		if ( pFlagsProperty && uiFlags == 0 ) uiFlags = pFlagsProperty->GetInteger();

		// Parse frequency
		const PBPropertyValue* pFreqProperty = pWaveProperties->GetProperty( "frequency" );
		if ( pFreqProperty )
		{
			if ( m_bSetFrequency )
				iFrequency = pFreqProperty->GetInteger();
			else
				iFrequency = ASoundManager::GetSingleton()->m_iGlobalFrequency;
		}

		// Parse length
		pWaveProperties->GetOptionalPropertyValue( fLength, "length" );

		vecWaves.EmplaceBack( uiFlags, iFrequency, fLength, TNULL, -1 );
		m_iNumWaves += 1;
	}

	// Allocate the actual array that will store info about samples
	m_pWaves = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) AWave[ m_iNumWaves ];

	// Copy data from the vector
	for ( TINT i = 0; i < m_iNumWaves; i++ )
		m_pWaves[ i ] = vecWaves[ i ];
}
