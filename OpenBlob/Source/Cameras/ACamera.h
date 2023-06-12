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
		Toshi::TVector3 m_vUnk;
		Toshi::TVector3 m_vUnk2;
	};

	enum CameraType
	{
		CT_TYPECOUNT = 4
	};

	ACamera(CameraType a_eCamType) : m_eCamType(a_eCamType)
	{
		InitCameraMatrix(m_Matrix);
		TASSERT(a_eCamType >= 0 && a_eCamType < CT_TYPECOUNT);
	}

	static void InitCameraMatrix(CameraMatrix camMat)
	{
		camMat.m_mMatrix.Identity();
		camMat.m_mShakeMatrix.Identity();
		camMat.m_vUnk = { 5.0f, 500.0f, sm_fDefaultFOV };
		camMat.m_vUnk2 = { Toshi::TMath::Tan(sm_fDefaultFOV * 0.5f), 0.5f, 0.f};
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

	CameraType m_eCamType;             // 0x8
	CameraMatrix m_Matrix;             // 0x6C
};

