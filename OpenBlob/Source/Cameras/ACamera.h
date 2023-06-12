#pragma once

class ACamera :
	public Toshi::TGenericClassDerived<ACamera, Toshi::TObject, "ACamera", TMAKEVERSION(1, 0), false>
{

	static constexpr float sm_fDefaultFOV = 1.0471976f;
	static const Toshi::TVector4 sm_vInitialPos;
	static const Toshi::TVector4 sm_vWorldUp;

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
		CT_TYPECOUNT = 4
	};

	ACamera(CameraType a_eCamType) : m_eCamType(a_eCamType)
	{
		ResetCameraMatrix(&m_Matrix);
		TASSERT(a_eCamType >= 0 && a_eCamType < CT_TYPECOUNT);
	}

	static void ResetCameraMatrix(CameraMatrix* camMat)
	{
		camMat->m_mMatrix.Identity();
		camMat->m_mShakeMatrix.Identity();
		camMat->m_fNear = 5.0f;
		camMat->m_fFar = 500.0f;
		camMat->m_fFOV = sm_fDefaultFOV;
		camMat->m_fAspect = Toshi::TMath::Tan(sm_fDefaultFOV * 0.5f);
		camMat->m_fProjectionCentreX = 0.5f;
		camMat->m_fProjectionCentreY = 0.0f;
	}

	void SetMatrix(const Toshi::TMatrix44& a_mMatrix)
	{
		m_Matrix.m_mMatrix = a_mMatrix;
	}

	void SetShakeMatrix(const Toshi::TMatrix44& a_mSkakeMatrix)
	{
		m_Matrix.m_mShakeMatrix = a_mSkakeMatrix;
	}

	void LookAtPoint(const Toshi::TVector4& a_vPoint);

	void LookAtDirection(const Toshi::TVector4& a_vDirection)
	{
		m_Matrix.m_mMatrix.LookAtDirection(a_vDirection, sm_vWorldUp);
	}

	void LookAtDirection(const Toshi::TVector4& a_vDirection, const Toshi::TVector4& a_vVec2)
	{
		m_Matrix.m_mMatrix.LookAtDirection(a_vDirection, a_vVec2);
	}

	CameraType m_eCamType;             // 0x8
	CameraMatrix m_Matrix;             // 0x6C
};

