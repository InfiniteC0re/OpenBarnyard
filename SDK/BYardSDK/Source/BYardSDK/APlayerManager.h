#pragma once
#include "THookedSingleton.h"
#include "ASteer.h"

class AUnitPlayer
    : public Toshi::TObject // TODO: replace with some unit class
{
public:
	ASteer* GetSteer() const { return m_pSteer; }

public:
	TBYTE PADDING1[ 24 ];
	ASteer* m_pSteer;
};

class APlayerManager
    : public Toshi::TObject
    , public THookedSingleton<APlayerManager, 0x007b48a8>
{
public:
	using PLAYER = TINT;
	enum PLAYER_ : PLAYER
	{
		PLAYER_1,
		PLAYER_2,
		PLAYER_3,
		PLAYER_4,
		PLAYER_NUMOF,
	};

public:
	AUnitPlayer* GetPlayer( PLAYER a_ePlayerIndex = PLAYER_1 );
};
