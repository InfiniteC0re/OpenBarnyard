#pragma once

class ABarnExploreMinigameState
{
public:
	ABarnExploreMinigameState() = delete;

	// The floor of the barn the player is on (ms_iFloor)
	using FLOOR = TINT;
	enum FLOOR_ : FLOOR
	{
		FLOOR_LOFT     = 0, // above y = 3.246
		FLOOR_GROUND   = 1,
		FLOOR_BASEMENT = 2, // below y = -4
	};

	static FLOOR GetFloor() { return *(const FLOOR*)( 0x00773a64 ); }
};
