#pragma once
#include "ASound.h"
#include "ASoundAdvanced.h"

#include <Toshi/T2DList.h>
#include <Toshi/TPString8.h>

class ASoundBank :
    public Toshi::T2DList<ASoundBank>::Node
{
public:
	friend class ASoundManager;

public:
	constexpr ASoundBank( TINT a_iNumSounds ) :
	    m_iNumSounds( a_iNumSounds ),
	    m_pSoundsEx( TNULL ),
	    m_pSounds( TNULL )
	{}

	~ASoundBank()
	{
		if ( m_pSoundsEx )
			delete[] m_pSoundsEx;

		if ( m_pSounds )
			delete[] m_pSounds;
	}

	void Load()
	{
		TIMPLEMENT();
	}

private:
	TINT             m_iNumSounds;
	Toshi::TPString8 m_strName;
	ASoundAdvanced*  m_pSoundsEx;
	ASound*          m_pSounds;
};
