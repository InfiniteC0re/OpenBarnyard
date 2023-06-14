#pragma once

class ACamera :
	public Toshi::TGenericClassDerived<ACamera, Toshi::TObject, "ACamera", TMAKEVERSION(1, 0), false>
{
public:
	struct CameraMatrix
	{
		Toshi::TMatrix44 m_mMatrix;
		Toshi::TMatrix44 m_mShakeMatrix;
		float m_fNear;
		float m_fFar;
		float m_fFOV;
		float m_fAspect;
		float m_fProjectionCentreX;
		float m_fProjectionCentreY;
	};

	enum CameraType
	{
		CT_DEFAULT,
		CT_BLOBFOLLOW,
		CT_SCRIPTED,
		CT_FREE,
		CT_KEYFRAME,
		CT_TYPECOUNT
	};

public:
	ACamera(CameraType a_eCamType) : m_eCamType(a_eCamType)
	{
		ResetCameraMatrix(m_Matrix);
		TASSERT(a_eCamType >= 0 && a_eCamType < CT_TYPECOUNT);
	}

	TBOOL IsInViewCone(const Toshi::TVector4& a_rvPos, float a_fSphereRadius) const;
	void LookAtPoint(const Toshi::TVector4& a_vPoint);
	
	static void ResetCameraMatrix(CameraMatrix& camMat);

	void RotateAroundWorldUp(float rotation)
	{
		RotateAroundAxis(sm_vWorldUp, rotation);
	}

	void RotateAroundAxis(const Toshi::TVector4& a_vAxis, float rotation);
	void RotateAroundRight(float rotation, float a_fVal);

	void SetMatrix(const Toshi::TMatrix44& a_mMatrix)
	{
		m_Matrix.m_mMatrix = a_mMatrix;
	}

	void SetShakeMatrix(const Toshi::TMatrix44& a_mSkakeMatrix)
	{
		m_Matrix.m_mShakeMatrix = a_mSkakeMatrix;
	}

	void LookAtDirection(const Toshi::TVector4& a_vDirection)
	{
		m_Matrix.m_mMatrix.LookAtDirection(a_vDirection, sm_vWorldUp);
	}

	void LookAtDirection(const Toshi::TVector4& a_vDirection, const Toshi::TVector4& a_vVec2)
	{
		m_Matrix.m_mMatrix.LookAtDirection(a_vDirection, a_vVec2);
	}

public:
	static constexpr float sm_fDefaultFOV = 1.0471976f;
	static const Toshi::TVector4 sm_vInitialPos;
	static const Toshi::TVector4 sm_vWorldUp;

protected:
	CameraType m_eCamType;             // 0x8
	CameraMatrix m_Matrix;             // 0x6C
};

