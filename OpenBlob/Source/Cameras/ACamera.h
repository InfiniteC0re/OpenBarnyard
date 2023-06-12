#pragma once

class ACamera :
	public Toshi::TGenericClassDerived<ACamera, Toshi::TObject, "ACamera", TMAKEVERSION(1, 0), false>
{

	static constexpr float sm_fDefaultFOV = 1.0471976f;
	static const Toshi::TVector4 sm_vInitialPos;
	static const Toshi::TVector4 sm_vWorldUp;

	enum CameraType
	{
		CT_TYPECOUNT = 4
	};

	ACamera(CameraType a_eCamType) : m_eCamType(a_eCamType)
	{
		InitMatrices(m_mMatrices);
		TASSERT(a_eCamType >= 0 && a_eCamType < CT_TYPECOUNT);
	}

	static void InitMatrices(Toshi::TMatrix44* matrices)
	{
		matrices[0].Identity();
		matrices[1].Identity();
		TTODO("...");
	}

	void SetMatrix(const Toshi::TMatrix44& a_mMatrix)
	{
		m_mMatrices[0] = a_mMatrix;
	}

	void SetShakeMatrix(const Toshi::TMatrix44& a_mSkakeMatrix)
	{
		m_mMatrices[1] = a_mSkakeMatrix;
	}

	void LookAtPoint(const Toshi::TVector4& a_vPoint);

	void LookAtDirection(const Toshi::TVector4& a_vDirection)
	{
		TIMPLEMENT();
	}

	CameraType m_eCamType;             // 0x8
	Toshi::TMatrix44 m_mMatrices[2];   // 0x6C
};

