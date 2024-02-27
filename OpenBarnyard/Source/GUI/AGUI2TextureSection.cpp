#include "pch.h"
#include "AGUI2TextureSection.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void AGUI2TextureSection::SetMaterial(AGUI2Material* a_pMaterial)
{
	m_pMaterial = a_pMaterial;
}

const TCHAR* AGUI2TextureSection::GetName() const
{
	return m_szName;
}

const TCHAR* AGUI2TextureSection::GetTextureFile() const
{
	return m_szTextureFile;
}

AGUI2Material* AGUI2TextureSection::GetMaterial()
{
	return m_pMaterial;
}

TVector4& AGUI2TextureSection::GetUVPoint1()
{
	return m_UV1;
}

TVector4& AGUI2TextureSection::GetUVPoint2()
{
	return m_UV2;
}

TFLOAT AGUI2TextureSection::GetWidth() const
{
	return TMath::Abs(TFLOAT(m_iPoint2X - m_iPoint1X));
}

TFLOAT AGUI2TextureSection::GetHeight() const
{
	return TMath::Abs(TFLOAT(m_iPoint2Y - m_iPoint1Y));
}

TINT AGUI2TextureSection::GetPoint1X() const
{
	return m_iPoint1X;
}

TINT AGUI2TextureSection::GetPoint1Y() const
{
	return m_iPoint1Y;
}

TINT AGUI2TextureSection::GetPoint2X() const
{
	return m_iPoint2X;
}

TINT AGUI2TextureSection::GetPoint2Y() const
{
	return m_iPoint2Y;
}
