#pragma once
#include "Cameras/ACamera.h"

class AFreeCamera : ACamera
{
	AFreeCamera() : ACamera(CT_FREECAM)
	{

	}

	void Yaw(float fYaw)
	{
		TTODO("Is it really -fYaw? Needs testing");
		RotateAroundWorldUp(-fYaw);
	}

	float m_fFreeCamSpeed; // 0x104
};

