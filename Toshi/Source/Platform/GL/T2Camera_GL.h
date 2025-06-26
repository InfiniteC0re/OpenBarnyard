#pragma once
#include "Render/TTransformObject.h"
#include "Render/T2RenderCommon.h"

TOSHI_NAMESPACE_START

class T2Camera
{
public:
	T2Camera();
	~T2Camera();

	TMatrix44 GetViewMatrix();

	CAMERAMODE GetCameraMode() const { return m_eCameraMode; }
	void       SetCameraMode( CAMERAMODE a_eCameraMode ) { m_eCameraMode = a_eCameraMode; }

	TFLOAT GetNearPlane() const { return m_fNearPlane; }
	TFLOAT GetFarPlane() const { return m_fFarPlane; }
	TFLOAT GetFOV() const { return m_fFOV; }

	void SetNearPlane( TFLOAT a_fValue ) { m_fNearPlane = a_fValue; }
	void SetFarPlane( TFLOAT a_fValue ) { m_fFarPlane = a_fValue; }
	void SetFOV( TFLOAT a_fValue ) { m_fFOV = a_fValue; }

	TTransformObject*       operator->() { return &m_TransformObject; }
	const TTransformObject* operator->() const { return &m_TransformObject; }

private:
	TFLOAT           m_fNearPlane;
	TFLOAT           m_fFarPlane;
	TFLOAT           m_fFOV;
	TTransformObject m_TransformObject;
	CAMERAMODE       m_eCameraMode;
};

TOSHI_NAMESPACE_END

