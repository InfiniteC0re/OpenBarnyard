#pragma once
#include "AFade.h"

TOBJECT(AFadeManager, Toshi::TTask, TTRUE),
	public Toshi::TSingleton<AFadeManager>
{
public:
	AFadeManager() = default;

	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;

	virtual void StopAllFades();

	AFade* CreateFade(const AFade::Color& a_rFadeFrom, const AFade::Color& a_rFadeTo, TFLOAT a_fFadeTime);

private:
	Toshi::T2ObjectPool<AFade, 8> m_Fades;
	Toshi::TQList<AFade> m_ActiveFades;
};
