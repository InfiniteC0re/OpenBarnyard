#pragma once
#include <cstdint>
#include "Toshi/Core/THPTimer.h"
#include "AAssetStreaming.h"


class AEntityLoader
{
	enum State
	{
		Loading,
		Creating,
		PostCreate,
		Done,
		IOLoading,
		LoadChallenge,
		DestructibleLoading,
		DestructibleCreating
	};

	struct ParamList
	{
		char* m_entityName;
		int m_iEntityCount;
	};

	State m_iState; // Globs 8 / deblob 4
	ParamList* m_pParamList; // Globs 12 / deblob 8
	int m_iCurEnt; // Globs 16 / deblob 12

	void SetState(State a_iState, const char* a_cStateName) { m_iState = a_iState; }
	void Update(float a_flt);
};