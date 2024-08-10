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

AWaveBank::AWaveBank( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strPath, TBOOL a_bSetFrequency )
{
	m_iNumWaves = 0;
	m_iNumLoads = 0;
	m_strBank = a_strBank;
	m_strPath = a_strPath;
	m_pWaves = TNULL;
	
	m_FileHandle = TNULL;
	m_bSetFrequency = a_bSetFrequency;

	ASoundManager::ms_WaveBanks.Insert( a_strBank, this );
}

AWaveSampleHandle AWaveBank::GetWaveSample( TINT a_iWaveIndex ) const
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	return m_pWaves[ a_iWaveIndex ].pSampleHandle;
}

void AWaveBank::Unknown( void* a_Unknown )
{

}

AWaveBank::~AWaveBank()
{
	ASoundManager::ms_WaveBanks.Remove( m_strBank );
	delete[] m_pWaves;
}

void AWaveBank::SetWaveFlag1( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	m_pWaves[ a_iWaveIndex ].uiFlags &= ~2;
	m_pWaves[ a_iWaveIndex ].uiFlags |= 1;
}

void AWaveBank::SetWaveFlag2( TINT a_iWaveIndex )
{
	TASSERT( a_iWaveIndex < m_iNumWaves );
	m_pWaves[ a_iWaveIndex ].uiFlags &= ~1;
	m_pWaves[ a_iWaveIndex ].uiFlags |= 2;
}

void AWaveBank::ParseWavesData( const PBProperties* a_pBankProperties, TUINT a_uiForcedFlags )
{
	TVALIDPTR( a_pBankProperties );

	m_iNumWaves = 0;

	T2DynamicVector<AWave> vecWaves( GetGlobalAllocator(), 128, 128 );

	for ( TUINT i = 0; i < a_pBankProperties->GetPropertyCount(); i++ )
	{
		const PBProperties::PBProperty* pProperty = a_pBankProperties->GetProperty( i );

		// Skip any properties other than Wave
		if ( pProperty->GetName().GetString()[ 0 ] != 'W' ) continue;

		TINT iFrequency = 22050;
		TFLOAT fLength = 0.0f;
		TUINT uiFlags = a_uiForcedFlags;

		const PBProperties* pWaveProperties = pProperty->GetValue()->GetProperties();

		// Parse flags
		const PBPropertyValue* pFlagsProperty = pWaveProperties->GetOptionalProperty( "flags" );
		if ( pFlagsProperty && uiFlags == 0 ) uiFlags = pFlagsProperty->GetInteger();

		// Parse frequency
		const PBPropertyValue* pFreqProperty = pWaveProperties->GetOptionalProperty( "frequency" );
		if ( pFreqProperty )
		{
			if ( m_bSetFrequency )
				iFrequency = pFreqProperty->GetInteger();
			else
				iFrequency = ASoundManager::GetSingleton()->m_iGlobalFrequency;
		}

		// Parse length
		const PBPropertyValue* pLengthProperty = pWaveProperties->GetOptionalProperty( "length" );
		if ( pLengthProperty )
		{
			if ( pLengthProperty->GetType() == PBPropertyValue::Type::Float )
				fLength = pLengthProperty->GetFloat();
			else
				fLength = TFLOAT( pLengthProperty->GetInteger() );
		}

		vecWaves.EmplaceBack( uiFlags, iFrequency, fLength, TNULL, -1 );
		m_iNumWaves += 1;
	}

	// Allocate the actual array that will store info about samples
	m_pWaves = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) AWave[ m_iNumWaves ];

	// Copy data from the vector
	for ( TINT i = 0; i < m_iNumWaves; i++ )
		m_pWaves[ i ] = vecWaves[ i ];
}
