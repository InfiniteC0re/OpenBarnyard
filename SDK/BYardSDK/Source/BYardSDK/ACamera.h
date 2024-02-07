#pragma once
#include "THookedSingleton.h"

#include <Math/TMatrix44.h>

struct ACamera
{
public:
	virtual ~ACamera();

public:
	Toshi::TMatrix44 m_Matrix;
	TFLOAT m_fFOV;
	TFLOAT m_fProjectionCentreX;
	TFLOAT m_fProjectionCentreY;
};

struct ACameraManager :
	public THookedSingleton<ACameraManager, 0x007822e0>
{
	ACamera* GetCurrentCamera()
	{
		return (ACamera*)((char*)this + 0x88);
	}
};