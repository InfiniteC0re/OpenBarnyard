#include "ToshiPCH.h"
#include "T2GUITextureSection.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

void T2GUITextureSection::SetMaterial( T2GUIMaterial* a_pMaterial )
{
	m_pMaterial = a_pMaterial;
}

const TCHAR* T2GUITextureSection::GetName() const
{
	return m_szName;
}

const TCHAR* T2GUITextureSection::GetTextureFile() const
{
	return m_szTextureFile;
}

T2GUIMaterial* T2GUITextureSection::GetMaterial() const
{
	return m_pMaterial;
}

const TVector4& T2GUITextureSection::GetUVPoint1() const
{
	return m_UV1;
}

const TVector4& T2GUITextureSection::GetUVPoint2() const
{
	return m_UV2;
}

TFLOAT T2GUITextureSection::GetWidth() const
{
	return TMath::Abs( TFLOAT( m_iPoint2X - m_iPoint1X ) );
}

TFLOAT T2GUITextureSection::GetHeight() const
{
	return TMath::Abs( TFLOAT( m_iPoint2Y - m_iPoint1Y ) );
}

TINT T2GUITextureSection::GetPoint1X() const
{
	return m_iPoint1X;
}

TINT T2GUITextureSection::GetPoint1Y() const
{
	return m_iPoint1Y;
}

TINT T2GUITextureSection::GetPoint2X() const
{
	return m_iPoint2X;
}

TINT T2GUITextureSection::GetPoint2Y() const
{
	return m_iPoint2Y;
}

void T2GUITextureSection::CalculateUV( TFLOAT a_flWidth, TFLOAT a_flHeight )
{
	m_UV1.x = TFLOAT( GetPoint1X() ) / a_flWidth;
	m_UV1.y = TFLOAT( GetPoint1Y() ) / a_flHeight;
	m_UV1.z = TFLOAT( GetPoint2X() ) / a_flWidth;
	m_UV1.w = TFLOAT( GetPoint1Y() ) / a_flHeight;

	m_UV2.x = TFLOAT( GetPoint2X() ) / a_flWidth;
	m_UV2.y = TFLOAT( GetPoint2Y() ) / a_flHeight;
	m_UV2.z = TFLOAT( GetPoint1X() ) / a_flWidth;
	m_UV2.w = TFLOAT( GetPoint2Y() ) / a_flHeight;
}

TOSHI_NAMESPACE_END
