#pragma once
#include "AGUI2Material.h"

#include <Toshi/Math/TVector4.h>
#include <Toshi/File/TTRB.h>

class AGUI2TextureSection
{
public:
	void SetMaterial(AGUI2Material* a_pMaterial) { m_pMaterial = a_pMaterial; }
	
	const char* GetName() const { return m_szName; }
	const char* GetTextureFile() const { return m_szTextureFile; }
	AGUI2Material* GetMaterial() { return m_pMaterial; }

	Toshi::TVector4& GetUVPoint1() { return m_UV1; }
	Toshi::TVector4& GetUVPoint2() { return m_UV2; }

	TFLOAT GetWidth() const
	{
		return Toshi::TMath::Abs(TFLOAT(m_iPoint2X - m_iPoint1X));
	}

	TFLOAT GetHeight() const
	{
		return Toshi::TMath::Abs(TFLOAT(m_iPoint2Y - m_iPoint1Y));
	}

	TINT GetPoint1X() const { return m_iPoint1X; }
	TINT GetPoint1Y() const { return m_iPoint1Y; }
	TINT GetPoint2X() const { return m_iPoint2X; }
	TINT GetPoint2Y() const { return m_iPoint2Y; }

private:
	const char* m_szName;
	const char* m_szTextureFile;
	TINT m_iPoint1X;
	TINT m_iPoint1Y;
	TINT m_iPoint2X;
	TINT m_iPoint2Y;
	AGUI2Material* m_pMaterial;
	Toshi::TVector4 m_UV1;
	Toshi::TVector4 m_UV2;
};

