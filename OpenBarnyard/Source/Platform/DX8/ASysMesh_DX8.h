#pragma once
#include "Render/Shader/ASysMesh.h"

TOBJECT(ASysMeshHAL, ASysMesh, TTRUE)
{
public:
	virtual TBOOL Render() override;
	virtual void SetZBias(TINT a_iZBias) override; // ?

public:
	inline static BOOL ms_bStopRendering;

private:
	TINT m_iZBias;
};
