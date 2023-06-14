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

	void Pitch(float fPitch)
	{
		TTODO("Is it really -fPitch? Needs testing");
		RotateAroundRight(-fPitch, 0.01f);
	}

	void SetFreeCameraSpeed(float a_fFreeCamSpeed)
	{
		TASSERT(m_fFreeCamSpeed > 0.0f, "Free Camera Speed must be larger than zero\n");
		m_fFreeCamSpeed = a_fFreeCamSpeed;
	}

	float m_fFreeCamSpeed; // 0x104
};

