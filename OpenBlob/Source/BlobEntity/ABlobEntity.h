#pragma once

#include "ABaseEntity.h"

class ABlobEntity : public ABaseEntity
{

	static constexpr int BLOB_SIZE_ABS_MAX = 10;

	enum State
	{
		STATE_ROLL =   0,
		STATE_LMARK =  4,
		STATE_BOSS =   5,
		STATE_CANNON = 7,
		STATE_DEAD =   11
	};

	void LoadProperties(bool reload);
};

