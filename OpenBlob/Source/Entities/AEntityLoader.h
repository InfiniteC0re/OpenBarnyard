#pragma once
#include "Toshi/File/TTRB.h"

class AEntityLoader
{
	enum State
	{
		STATE_Loading,
		STATE_Creating,
		STATE_PostCreate,
		STATE_Done,
		STATE_IOLoading,
		STATE_LoadChallenge,
		STATE_DestructibleLoading,
		STATE_DestructibleCreating
	};

	struct ParamList
	{
		char* m_entityName;
		int m_iEntityCount;
	};

	Toshi::TTRB* m_pTRB;      // 0x0
	State m_iState;           // 0x4
	ParamList* m_pParamList;  // 0x8
	int m_iCurEnt;            // 0x12

	void SetState(State a_iState, const char* a_cStateName = TNULL) { m_iState = a_iState; }
	void Update(float a_flt);
};