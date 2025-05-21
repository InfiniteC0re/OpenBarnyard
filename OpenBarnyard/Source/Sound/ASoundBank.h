#pragma once
#include "ASoundLegacy.h"
#include "ASound.h"

#include <Toshi/T2DList.h>
#include <Toshi/TPString8.h>

class ASoundBank : public Toshi::T2DList<ASoundBank>::Node
{
public:
	friend class ASoundManager;

public:
	constexpr ASoundBank( TINT a_iNumSounds )
	    : m_iNumSounds( a_iNumSounds ), m_pSoundsEx( TNULL ), m_pSounds( TNULL )
	{}

	~ASoundBank()
	{
		if ( m_pSoundsEx )
			delete[] m_pSoundsEx;

		if ( m_pSounds )
			delete[] m_pSounds;
	}

	// $Barnyard: FUNCTION 005d6580
	void Load()
	{
		for ( TINT i = 0; i < m_iNumSounds; i++ )
		{
			for ( TINT k = 0; k < m_pSoundsEx[ i ].GetSamples().Size(); k++ )
			{
				ASound::Sample& rSample = m_pSoundsEx[ i ].GetSamples()[ k ];
				rSample.iFrequency      = TINT( rSample.pWaveBank->GetWaveFrequency( rSample.iId ) * rSample.fPitch );
			}
		}
	}

private:
	TINT             m_iNumSounds;
	Toshi::TPString8 m_strName;
	ASound*          m_pSoundsEx;
	ASoundLegacy*    m_pSounds;
};
