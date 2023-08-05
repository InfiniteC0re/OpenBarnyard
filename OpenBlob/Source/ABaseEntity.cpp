#include "pch.h"
#include "ABaseEntity.h"

void ABaseEntity::SetState(int a_State)
{
	TASSERT(a_State >= 0);
	if (a_State != m_State)
	{
		OnLeaveState(m_State);
		m_State = a_State;
		m_fTicks = 0.0f;
		OnEnterState(m_State);
	}
}
