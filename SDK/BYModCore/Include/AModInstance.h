#pragma once
#include <Toshi2/T2DList.h>

class AModInstance :
	public Toshi::T2DList<AModInstance>::Node
{
public:
	using t_UpdateMod = TBOOL(*)(TFLOAT a_fDeltaTime);

public:
	AModInstance(t_UpdateMod a_fnUpdateCallback) :
		m_fnUpdateCallback(a_fnUpdateCallback)
	{ }

	TBOOL Update(TFLOAT a_fDeltaTime)
	{
		if (m_fnUpdateCallback)
		{
			return m_fnUpdateCallback(a_fDeltaTime);
		}

		return TTRUE;
	}

private:
	t_UpdateMod m_fnUpdateCallback;
};
