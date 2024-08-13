#pragma once
#include "AGUI2Material.h"

#include <Math/TVector4.h>
#include <File/TTRB.h>

class AGUI2TextureSection
{
public:
	void SetMaterial( AGUI2Material* a_pMaterial );

	const TCHAR*   GetName() const;
	const TCHAR*   GetTextureFile() const;
	AGUI2Material* GetMaterial();

	Toshi::TVector4& GetUVPoint1();
	Toshi::TVector4& GetUVPoint2();

	TFLOAT GetWidth() const;
	TFLOAT GetHeight() const;

	TINT GetPoint1X() const;
	TINT GetPoint1Y() const;
	TINT GetPoint2X() const;
	TINT GetPoint2Y() const;

private:
	const TCHAR*	m_szName;
	const TCHAR*	m_szTextureFile;
	TINT			m_iPoint1X;
	TINT			m_iPoint1Y;
	TINT			m_iPoint2X;
	TINT			m_iPoint2Y;
	AGUI2Material*	m_pMaterial;
	Toshi::TVector4 m_UV1;
	Toshi::TVector4 m_UV2;
};
