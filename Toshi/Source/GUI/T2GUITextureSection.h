#pragma once
#include "T2GUIMaterial.h"

#include <Math/TVector4.h>
#include <File/TTRB.h>

TOSHI_NAMESPACE_START

class T2GUITextureSection
{
public:
	T2GUITextureSection()  = default;
	~T2GUITextureSection() = default;

	void SetMaterial( T2GUIMaterial* a_pMaterial );

	const TCHAR*   GetName() const;
	const TCHAR*   GetTextureFile() const;
	T2GUIMaterial* GetMaterial() const;

	const TVector4& GetUVPoint1() const;
	const TVector4& GetUVPoint2() const;

	TFLOAT GetWidth() const;
	TFLOAT GetHeight() const;

	TINT GetPoint1X() const;
	TINT GetPoint1Y() const;
	TINT GetPoint2X() const;
	TINT GetPoint2Y() const;

	void CalculateUV( TFLOAT a_flWidth, TFLOAT a_flHeight );

private:
	const TCHAR*   m_szName;
	const TCHAR*   m_szTextureFile;
	TINT           m_iPoint1X;
	TINT           m_iPoint1Y;
	TINT           m_iPoint2X;
	TINT           m_iPoint2Y;
	T2GUIMaterial* m_pMaterial;
	TVector4       m_UV1;
	TVector4       m_UV2;
};

TOSHI_NAMESPACE_END
