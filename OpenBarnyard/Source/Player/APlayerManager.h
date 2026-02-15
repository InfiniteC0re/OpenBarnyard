#pragma once
#include "APlayerSet.h"

#include <Toshi/TObject.h>

class APlayerManager
    : public Toshi::TObject
    , public Toshi::TSingleton<APlayerManager>
{
public:
	APlayerManager();
	~APlayerManager();

	APlayerSet* GetPlayerSet( TINT a_eType )
	{
		TASSERT( a_eType < TARRAYSIZE( m_aPlayerSets ) );
		return &m_aPlayerSets[ a_eType ];
	}

	void ResetPlayerIterator() { m_iCurrentPlayerIt = 0; }

private:
	APlayerSet* m_pPlayerSets;
	APlayerSet  m_aPlayerSets[ 2 ];

	// ...

	TINT m_iCurrentPlayerIt;
};
